# Tool Features

## Available Memory Models

By default, GenMC verifies programs under RC11. However, apart from
RC11, GenMC also supports other models like SC and IMM. The difference
between these memory models (as far as allowed outcomes are concerned)
can be seen in the following program:

```c
/* file: lb.c */
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <assert.h>

atomic_int x;
atomic_int y;

void *thread_1(void *unused)
{
    int a = atomic_load_explicit(&x, memory_order_relaxed);
    atomic_store_explicit(&y, 1, memory_order_relaxed);
    return NULL;
}

void *thread_2(void *unused)
{
    int b = atomic_load_explicit(&y, memory_order_relaxed);
    atomic_store_explicit(&x, 1, memory_order_relaxed);
    return NULL;
}

int main()
{
    pthread_t t1, t2;

    if (pthread_create(&t1, NULL, thread_1, NULL))
        abort();
    if (pthread_create(&t2, NULL, thread_2, NULL))
        abort();

    return 0;
}
```

Under RC11, an execution where both `a` = 1 and `b` = 1 is forbidden,
whereas such an execution is allowed under IMM. To account for such
behaviors, GenMC tracks dependencies between program instructions thus
leading to a constant overhead when verifying programs under models
like IMM.

### Note on Language Memory Models vs Hardware Memory Models

RC11 is a language-level memory model while IMM is a hardware memory
model. Subsequently, the verification results produced by GenMC for
the two models should be interpreted somewhat differently.

What this means in practice is that, when verifying programs under
RC11, the input file is assumed to be the very source code the user
wrote. A successful verification result under RC11 holds all the way
down to the actual executable, due to the guarantees provided by RC11
[\[Lahav et al. 2017\]](references.md#lahav2017repairing).

On the other hand, when verifying programs under IMM, the input file
is assumed to be the assembly code run by the processor (or, more
precisely, a program in IMM's intermediate language). And while GenMC
allows the input file to be a C file (as in the case of RC11), it
assumes that this C file corresponds to an assembly file that is the
result of the compilation of some program in IMM's language. In other
words, program correctness is not preserved across compilation for IMM
inputs.

## Race Detection and Memory Errors

For memory models that define the notion of a race, GenMC will report
executions containing races as erroneous. For instance, under RC11,
the following program is racy, as there is no happens-before between
the write of `x` in the first thread and the non-atomic read of `x` in
the second thread (even though the latter causally depends on the
former).

```c
/* file: race.c */
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <assert.h>

atomic_int x;

void *thread_1(void *unused)
{
    atomic_store_explicit(&x, 1, memory_order_relaxed);
    return NULL;
}

void *thread_2(void *unused)
{
    int a, b;

    a = atomic_load_explicit(&x, memory_order_relaxed);
    if (a == 1)
        b = *((int *) &x);
    return NULL;
}

int main()
{
    pthread_t t1, t2;

    if (pthread_create(&t1, NULL, thread_1, NULL))
        abort();
    if (pthread_create(&t2, NULL, thread_2, NULL))
        abort();

    return 0;
}
```

Additionally, for all memory models, GenMC detects some memory races
like accessing memory that has been already freed, accessing dynamic
memory that has not been allocated, or freeing an already freed chunk
of memory.

Race detection can be completely disabled by means of
`-disable-race-detection`, which may yield better performance for
certain programs.

## Barrier-Aware Model Checking (BAM)

GenMC v0.6 comes with built-in support for `pthread_barrier_t`
functions (see [Supported APIs](api.md)) via BAM [\[Kokologiannakis and
Vafeiadis 2021\]](references.md#kokologiannakis2021bam). As an example
of BAM in action, consider the following program:

```c
/* file: bam.c */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <genmc.h>
#include <assert.h>

#ifndef N
# define N 2
#endif

pthread_barrier_t b;
atomic_int x;

void *thread_n(void *unused)
{
    ++x;
    pthread_barrier_wait(&b);
    assert(x == N);
    return NULL;
}

int main()
{
    pthread_t t[N];

    pthread_barrier_init(&b, NULL, N);
    for (int i = 0u; i < N; i++) {
        if (pthread_create(&t[i], NULL, thread_n, NULL))
            abort();
    }

    return 0;
}
```

Running GenMC on the program above results in the following output:

```sh
Number of complete executions explored: 2
Total wall-clock time: 0.01s
```

As can be seen, GenMC treats `barrier_wait` calls as no-ops, and they
do not lead to any additional explorations. (The two executions
explored correspond to the possible ways in which `x` can be
incremented).

However, if we disable BAM by means of the `-disable-bam` switch, we
get the following output:

```sh
Number of complete executions explored: 4
Number of blocked executions seen: 4
Total wall-clock time: 0.01s
```

Note that while BAM can lead to the exploration of exponentially fewer
executions, it can only be used if the result of the `barrier_wait` is
not used. If it is, then using `-disable-bam` is necessary, as GenMC
currently does not enforce this limitation.

## State-Space Bounding

Under SC, GenMC can bound the state-space exploration using either
preemption bounding [\[Marmanis et
al. 2023a\]](references.md#marmanis2023buster) and round-robin bounding
[\[Marmanis and Vafeiadis
2023b\]](references.md#marmanis2023robin-bound).

For instance, in the following program, running GenMC with `--sc
--bound=0 --bound-type=context` will avoid exploring executions that
have one or more (preemptive) context-switches.

```c
/* file: bound.c */
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

atomic_int x;

void * thread_1(void * unused)
{
    atomic_store(&x, 1);
    atomic_store(&x, 2);
    return NULL;
}

void * thread_2(void * unused)
{
    atomic_load(&x);
    return NULL;
}

int main()
{
    pthread_t t1, t2;

    if (pthread_create(&t1, NULL, thread_1, NULL))
        abort();
    if (pthread_create(&t2, NULL, thread_2, NULL))
        abort();

    return 0;
}
```

To guarantee that no execution within the bound is missed, some
executions that exceed the bound might also be explored, and are
reported appropriately:

```sh
Number of complete executions explored: 3 (1 exceeded bound)
```

The default bounding type (`round`), on the other hand, only explores
executions within the given bound. For instance, when running GenMC
with `--sc --bound=0`, only the single SC execution that can be
obtained with zero rounds (i.e., in one go) using a left-to-right
round-robin scheduler will be explored.

```sh
Number of complete executions explored: 1
```

Note that, while the round-robin bound does not explore executions
that exceed the limit, the number of executions grows more rapidly as
the bound increases (compared to context bounding).

## Symmetry Reduction

GenMC also employs an experimental symmetry reduction mechanism
[\[Kokologiannakis et
al. 2024\]](references.md#kokologiannakis2024spore), which is
beneficial to use when threads are running the same code.

For instance, in the following program, GenMC explores only one execution instead of 6.

```c
/* file: sr.c */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <genmc.h>
#include <assert.h>

atomic_int x;

void *thread_n(void *unused)
{
    ++x;
    return NULL;
}

int main()
{
    pthread_t t1, t2, t3;

    if (pthread_create(&t1, NULL, thread_n, NULL))
        abort();
    if (pthread_create(&t2, NULL, thread_n, NULL))
        abort();
    if (pthread_create(&t3, NULL, thread_n, NULL))
        abort();

    return 0;
}
```

In order for symmetry reduction to actually take place, the spawned
threads need to share exactly the same code, have exactly the same
arguments, and also there must not be any memory access (at the
LLVM-IR level) between the spawn instructions.

To make GenMC use symmetry reduction, one can use the primitive
`__VERIFIER_spawn_symmetric(fun, tid)` (defined in `genmc.h`), the
last argument of which is the thread identifier of the last
(previously spawned) symmetric predecessor.

## Checking Liveness

GenMC can also check for liveness violations in programs with
spinloops. Consider the following simple program:

```c
/* file: liveness.c */
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>

atomic_int x;

void *thread_1(void *unused)
{
    while (!x)
        ;
    return NULL;
}

int main()
{
    pthread_t t1;

    if (pthread_create(&t1, NULL, thread_1, NULL))
        abort();

    return 0;
}
```

Since there are no writes to `x`, the loop in `thread_1` above will
never terminate. Indeed, running GenMC with `-check-liveness` produces
a relevant error report:

```sh
Error detected: Liveness violation!
Event (1, 4) in graph:
<-1, 0> main:
    (0, 0): B
    (0, 1): TC [forks 1] L.19
    (0, 2): E
<0, 1> thread_1:
    (1, 0): B
    (1, 1): LOOP_BEGIN
    (1, 2): SPIN_START
    (1, 3): Rsc (x, 0) [INIT] L.9
    (1, 4): BLOCK [spinloop]

Non-terminating spinloop: thread 1
Number of complete executions explored: 0
Number of blocked executions seen: 1
Total wall-clock time: 0.07s
```

The `-check-liveness` switch will automatically check for liveness
violations in all loops that have been captured by the spin-assume
transformation.

## Checking Linearizability

GenMC implements the Relinche algorithm [\[Golovin et
al. 2025\]](references.md#goloving2025relinche) for checking (bounded)
linearizability. This algorithm requires using GenMC with the "Most
Parallel Client" (MPC). Such a client for queues and stacks is already
provided in GenMC's test suite (e.g.,
`tests/correct/relinche/queue/mpc.c`).

To use the client e.g., for a queue, we have to provide a queue
implementation that defines the following methods: `init_queue()`,
`enqueue()`, `dequeue()` and `clear_queue()`. For instance, an
implementation of a Herlihy-Wing queue can be seen below:

```c
/* file: hw-queue.c */
#include <stdatomic.h>
#include <assert.h>
#include <genmc.h>
#include <stdbool.h>

#define MAX_NODES    0xff

typedef struct _queue_t {
    _Atomic(int) tail;
    _Atomic(unsigned int) nodes[MAX_NODES] ;
} queue_t;

void init_queue(queue_t *q, int num_threads)
{
}

void clear_queue(queue_t *q, int num_threads)
{
}

void enqueue(queue_t *q, unsigned int val)
{
    int i = atomic_fetch_add_explicit(&q->tail, 1, release);
    assert(i + 1 < MAX_NODES);
    atomic_store_explicit(&q->nodes[i + 1], val, release);
}

bool dequeue(queue_t *q, unsigned int *ret)
{
    bool success = false;
    while (!success) {
        int tail = atomic_load_explicit(&q->tail, acquire);
        for (int i = 0; i <= tail; ++i) {
            if (atomic_load_explicit(&q->nodes[i], acquire) == 0)
                continue;
#ifdef BUG /* Linearizability bug */
            unsigned int v = atomic_exchange_explicit(&q->nodes[i], 0, acquire);
#else
            unsigned int v = atomic_exchange_explicit(&q->nodes[i], 0, acq_rel);
#endif
            if (v != 0) {
                *ret = v;
                success = true;
                break;
            }
        }
        __VERIFIER_assume(success);
    }
    return *ret;
}
```

To check correctness of the above implementation, we first have to
provide a specification. GenMC can produce such a specification file
from a reference implementation, but we can also use one of the
predefined specification files (e.g.,
`tests/correct/queue/queue_spec_rc11.in`) as follows:

```sh
genmc -rc11 -disable-mm-detector --check-lin-spec=spec.in -- -DRTN=2 -DWTN=2 -include hw-queue.c mpc.c"
```

Doing so with the above implementation and spec file, will check for
linearizability of all clients with two `dequeue` and two `enqueue`
operations. GenMC produces the following output:

```sh
GenMC v0.10.3 (LLVM 16.0.6)
Copyright (C) 2024 MPI-SWS. All rights reserved.

*** Compilation complete.
*** Transformation complete.
Tip: Estimating state-space size. For better performance, you can use --disable-estimation.
*** Estimation complete.
Total executions estimate: 33 (+- 37)
Time to completion estimate: 0.03s
*** Verification complete.
No errors were detected.
Number of complete executions explored: 6
Number of blocked executions seen: 10
Number of checked hints: 1
Relinche time: 0.00s
Total wall-clock time: 0.03s
```
