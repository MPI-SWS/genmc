
# Table of Contents

1.  [Introduction ](#org795e6fb)
2.  [Basic Usage](#org700fd27)
    1.  [A First Example](#org7e27d8c)
    2.  [Reducing the State Space of a Program With `assume()` Statements ](#orgb09a697)
    3.  [Handling Infinite Loops ](#org4c56a6b)
    4.  [Error Reporting ](#org9d91cfd)
3.  [Tool Features ](#org0589613)
    1.  [Available Memory Models ](#org7cc84c4)
        1.  [Note on LKMM Support](#orgc9035a4)
        2.  [Note on Language Memory Models vs Hardware Memory Models](#org840ad38)
    2.  [Race Detection and Memory Errors](#org3d3b7e5)
    3.  [Lock-Aware Partial Order Reduction (<font style="font-variant: small-caps">LAPOR</font>)](#orgf6357b1)
    4.  [Barrier-Aware Model Checking (<font style="font-variant: small-caps">BAM</font>)](#org20cc1f4)
    5.  [Symmetry Reduction](#orga4022a7)
    6.  [Checking Liveness ](#org8879883)
    7.  [System Calls and Persistency Checks (<font style="font-variant: small-caps">Persevere</font>) ](#orgf475797)
        1.  [Consistency of File-Manipulating Programs](#orgc7d221d)
        2.  [Persistency of File-Manipulating Programs](#org1ddc7c7)
4.  [Command-line Options ](#org2b0335c)
5.  [Supported APIs ](#org7e9d263)
    1.  [Supported `stdio`, `unistd` and `fcntl` API](#org3ec10dd)
    2.  [Supported `stdlib` API](#org55d405b)
    3.  [Supported `pthread` API](#orgac7c1f9)
    4.  [Supported SV-COMP (SV-COMP 2019) API](#org09067a4)
6.  [Contact ](#org77d7590)



<a id="org795e6fb"></a>

# Introduction <a id="org8fc088c"></a>

<font style="font-variant: small-caps">GenMC</font> is an effective stateless model checker for programs
written under the RC11 (Lahav et al. 2017), IMM
(Podkopaev, Lahav, and Vafeiadis 2019), and LKMM (Alglave et al. 2018) memory models.
<font style="font-variant: small-caps">GenMC</font> verifies safety properties of C programs that use `C11`
atomics and the `pthread` library for concurrency. It employs a very
effective dynamic partial order reduction technique
(Kokologiannakis, Raad, and Vafeiadis 2019a, 2022) that is
sound, complete and optimal.

<font style="font-variant: small-caps">GenMC</font> works at the level of LLVM Intermediate Representation (LLVM-IR)
and uses `clang` to translate C programs to LLVM-IR. This means it
can miss some bugs that are removed during the translation to LLVM-IR,
but it is guaranteed to encounter at least as many bugs as the
compiler backend will encounter.

<font style="font-variant: small-caps">GenMC</font> should compile on Linux and Mac OSX provided that the relevant
dependencies are installed.


<a id="org700fd27"></a>

# Basic Usage

A generic invocation of <font style="font-variant: small-caps">GenMC</font> resembles the following:

    genmc [OPTIONS] -- [CFLAGS] <file>

In the above command, `OPTIONS` include several options that can be
passed to <font style="font-variant: small-caps">GenMC</font> (see Section [4](#org53d284e) for more details), and
`CFLAGS` are the options that one would normally pass to the C
compiler. If no such flags exist, the `--` can be omitted.
Lastly, `file` should be a C file that uses the `stdatomic.h`
and `pthread.h` APIs for concurrency.

Note that, in order for <font style="font-variant: small-caps">GenMC</font> to be able to verify it, `file`
needs to meet two requirements: finiteness and data-determinism.
Finiteness means that all tests need to have finite traces,
i.e., no infinite loops (these need to be bounded; see
Section [2.3](#orgaa0e9d1)). Data-determinism means that
the code under test should be data-deterministic, i.e.,
not perform actions like calling `rand()`, performing
actions based on user input or the clock, etc. In other words,
all non-determinism should originate either from the scheduler
or the underlying (weak) memory model.

As long as these requirements as satisfied, <font style="font-variant: small-caps">GenMC</font> will detect safety
errors, races on non-atomic variables, as well as some memory errors
(e.g., double-free error). Users can provide safety specifications for
their programs by using `assert()` statements.


<a id="org7e27d8c"></a>

## A First Example

Consider the following program, demonstrating the Message-Passing (MP)
idiom:

    /* file: mp.c */
    #include <stdlib.h>
    #include <pthread.h>
    #include <stdatomic.h>
    #include <stdbool.h>
    #include <assert.h>

    atomic_int data;
    atomic_bool ready;

    void *thread_1(void *unused)
    {
            atomic_store_explicit(&data, 42, memory_order_relaxed);
            atomic_store_explicit(&ready, true, memory_order_release);
            return NULL;
    }

    void *thread_2(void *unused)
    {
            if (atomic_load_explicit(&ready, memory_order_acquire)) {
                    int d = atomic_load_explicit(&data, memory_order_relaxed);
                    assert(d == 42);
            }
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

In order to analyze the code above with <font style="font-variant: small-caps">GenMC</font>, we can use the
following command:

    genmc mp.c

with which <font style="font-variant: small-caps">GenMC</font> will yield the following result:

    Number of complete executions explored: 2
    Total wall-clock time: 0.02s

<font style="font-variant: small-caps">GenMC</font> explores two executions: one where $ready = data =0$, and
one where $ready = data = 1$.


<a id="orgb09a697"></a>

## Reducing the State Space of a Program With `assume()` Statements <a id="org14dc8a6"></a>

In some programs, we only care about what happens when certain reads
read certain values of interest. That said, by default, <font style="font-variant: small-caps">GenMC</font>
will explore all possible values for all program loads, possibly
leading to the exploration of an exponential number of executions.

To alleviate this problem, <font style="font-variant: small-caps">GenMC</font> supports the
`__VERIFIER_assume()` function (similar to the one specified in
SV-COMP (SV-COMP 2019)). This function takes an integer argument
(e.g., the value read from a load), and only continues the execution
if the argument is non-zero.

For example, let us consider the MP program from the previous section,
and suppose that we are only interested in verifying the assertion
in cases where the first read of the second thread reads 1. We can
use an `assume()` statement to achieve this, as shown below:

    /* file: mp-assume.c */
    #include <stdlib.h>
    #include <pthread.h>
    #include <stdatomic.h>
    #include <stdbool.h>
    #include <assert.h>

    void __VERIFIER_assume(int);

    atomic_int data;
    atomic_bool ready;

    void *thread_1(void *unused)
    {
            atomic_store_explicit(&data, 42, memory_order_relaxed);
            atomic_store_explicit(&ready, true, memory_order_release);
            return NULL;
    }

    void *thread_2(void *unused)
    {
            int r = atomic_load_explicit(&ready, memory_order_acquire);
            __VERIFIER_assume(r);
            if (r) {
                    int d = atomic_load_explicit(&data, memory_order_relaxed);
                    assert(d == 42);
            }
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

Note that the `__VERIFIER_assume()` function has to be declared. Alternatively,
one can include the `<genmc.h>` header, that contains the declarations for all
the special function that <font style="font-variant: small-caps">GenMC</font> offers (see Section [5](#org801c94c)).

If we run <font style="font-variant: small-caps">GenMC</font> on the `mp-assume.c` program above, we get the following
output:

    Number of complete executions explored: 1
    Number of blocked executions seen: 1
    Total wall-clock time: 0.02s

As can be seen, <font style="font-variant: small-caps">GenMC</font> only explored one full execution (the one
where $r = 1$, while the execution where $r = 0$ was blocked, because
of the `assume()` statement. Of course, while the usage of `assume()`
does not make any practical difference in this small example, this is
not always the case: generally, using `assume()` might yield an
exponential improvement in <font style="font-variant: small-caps">GenMC</font>'s running time.

Finally, note that, when using <font style="font-variant: small-caps">GenMC</font> under memory models that
track dependencies (see Section [3.1](#org0b70d16)), an `assume()`
statement will introduce a control dependency in the program code.


<a id="org4c56a6b"></a>

## Handling Infinite Loops <a id="orgaa0e9d1"></a>

As mentioned in the beginning of this section, all programs that
<font style="font-variant: small-caps">GenMC</font> can handle need to have finite traces. That said, many
programs of interest do not fulfill this requirement, because, for
example, they have some infinite loop. <font style="font-variant: small-caps">GenMC</font> offers three
solutions for such cases.

First, <font style="font-variant: small-caps">GenMC</font> can automatically perform the "spin-assume"
transformation for a large class of spinloops. Specifically, as long
as a spinloop completes a full iteration with no visible side effects
(e.g., stores to global variables), <font style="font-variant: small-caps">GenMC</font> will cut the respective
execution. For instance, consider the following simple loop:

    int r = 0;
    while (!atomic_compare_exchange_strong(&x, &r, 1))
            r = 0;

Since this loop has no visible side-effects whenever it completes
a full iteration, <font style="font-variant: small-caps">GenMC</font> will not explore more than one
execution where the loop fails (the execution where the loop fails
will be reported as a blocked execution). The "spin-assume"
transformation has proven to be very effective for a wide range of
loops; for more details on whether it applies on a specific loop,
please see (Kokologiannakis, Ren, and Vafeiadis 2021).

Finally, for infinite loops with side effects, we can use the
`-unroll=N` command-line option (see Section [4](#org53d284e)). This option
bounds all loops so that they are executed at most `N` times.  In this
case, any verification guarantees that <font style="font-variant: small-caps">GenMC</font> provides hold up to
that bound.  If you are unsure whether you should use the `-unroll=N`
switch, you can try to verify the program and check whether
<font style="font-variant: small-caps">GenMC</font> complains about the graph size
(`-warn-on-graph-size=<N>`). If it does, there is a good chance you
need to use the `-unroll=N` switch.

Note that the loop-bounding happens at the LLVM-IR level, which means
that the loops there may not directly correspond to loops in the C
code (depending on the enabled compiled optimizations, etc).


<a id="org9d91cfd"></a>

## Error Reporting <a id="orgf1467ce"></a>

In the previous sections, saw how <font style="font-variant: small-caps">GenMC</font> verifies the small MP program.
Let us now proceed with an erroneous version of this program, in order
to show how <font style="font-variant: small-caps">GenMC</font> reports errors to the user.

Consider the following variant of the MP program below, where the
store to `ready` in the first thread is now performed using a relaxed
access:

    /* file: mp-error.c */
    #include <stdlib.h>
    #include <pthread.h>
    #include <stdatomic.h>
    #include <stdbool.h>
    #include <assert.h>

    atomic_int data;
    atomic_bool ready;

    void *thread_1(void *unused)
    {
            atomic_store_explicit(&data, 42, memory_order_relaxed);
            atomic_store_explicit(&ready, true, memory_order_relaxed);
            return NULL;
    }

    void *thread_2(void *unused)
    {
            if (atomic_load_explicit(&ready, memory_order_acquire)) {
                    int d = atomic_load_explicit(&data, memory_order_relaxed);
                    assert(d == 42);
            }
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

This program is buggy since the load from `ready` no longer
synchronizes with the corresponding store, which in turn means that
the load from `data` may also read 0 (the initial value), and
not just 42.

Running <font style="font-variant: small-caps">GenMC</font> on the above program, we get the following outcome:

    Error detected: Safety violation!
    Event (2, 2) in graph:
    <-1, 0> main:
            (0, 0): B
            (0, 1): M
            (0, 2): M
            (0, 3): TC [forks 1] L.30
            (0, 4): Wna (t1, 1) L.30
            (0, 5): TC [forks 2] L.32
            (0, 6): Wna (t2, 2) L.32
            (0, 7): E
    <0, 1> thread_1:
            (1, 0): B
            (1, 1): Wrlx (data, 42) L.12
            (1, 2): Wrlx (ready, 1) L.13
            (1, 3): E
    <0, 2> thread_2:
            (2, 0): B
            (2, 1): Racq (ready, 1) [(1, 2)] L.19
            (2, 2): Rrlx (data, 0) [INIT] L.20

    Assertion violation: d == 42
    Number of complete executions explored: 1
    Total wall-clock time: 0.02s

<font style="font-variant: small-caps">GenMC</font> reports an error and prints some information relevant for
debugging. First, it prints the type of the error, then the execution
graph representing the erroneous execution, and finally the error
message, along with the executions explored so far and the time that
was required.

The graph contains the events of each thread along with some
information about the corresponding source-code instructions.  For
example, for write events (e.g., event (1, 1)), the access mode, the
name of the variable accessed, the value written, as well as the
corresponding source-code line are printed. The situation is similar
for reads (e.g., event (2, 1)), but also the position in the graph
from which the read is reading from is printed.

Note that there are many different types of events. However, many of
them are <font style="font-variant: small-caps">GenMC</font>-related and not of particular interest to users (e.g.,
events labeled with \`B', which correspond to the beginning of a
thread). Thus, <font style="font-variant: small-caps">GenMC</font> only prints the source-code lines for events
that correspond to actual user instructions, thus helping the
debugging procedure.

Finally, when more information regarding an error are required,
two command-line switches are provided. The `-dump-error-graph=<file>`
switch provides a visual representation of the erroneous execution,
as it will output the reported graph in DOT format in `<file>`,
so that it can be viewed by a PDF viewer. Finally, the `-print-error-trace`
switch will print a sequence of source-code lines leading to
the error. The latter is especially useful for cases where
the bug is not caused by some weak-memory effect but rather from
some particular interleaving (e.g., if all accesses are
 `memory_order_seq_cst`), and the write where each read is reading
from can be determined simply by locating the previous write in the
same memory location in the sequence.


<a id="org0589613"></a>

# Tool Features <a id="org3a48379"></a>


<a id="org7cc84c4"></a>

## Available Memory Models <a id="org0b70d16"></a>

By default, <font style="font-variant: small-caps">GenMC</font> verifies programs under RC11. However, apart
from RC11, <font style="font-variant: small-caps">GenMC</font> also supports IMM and (experimentally) LKMM.
The difference between these memory models (as far as allowed outcomes are concerned)
can be seen in the following program:

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

Under RC11, an execution where both $a = 1$ and $b = 1$ is forbidden,
whereas such an execution is allowed under IMM and LKMM. To account for such
behaviors, <font style="font-variant: small-caps">GenMC</font> tracks dependencies between program instructions
thus leading to a constant overhead when verifying programs under
models like IMM.


<a id="orgc9035a4"></a>

### Note on LKMM Support

<font style="font-variant: small-caps">GenMC</font>'s support for LKMM is currently at an experimental stage.
<font style="font-variant: small-caps">GenMC</font> includes plain accesses in <span style="color: #808080; font-family: monospace">ppo</span>
(in contrast to what (Alglave et al. 2018) dictates), as plain accesses
to temporary LLVM-IR variables are occasionally generated by `clang` between
accesses to shared memory, and thus including them in <span style="color: #808080; font-family: monospace">ppo</span> is
necessary to preserve dependencies.

Tests that use LKMM atomics need to include `<lkmm.h`.


<a id="org840ad38"></a>

### Note on Language Memory Models vs Hardware Memory Models

RC11 is a language-level memory model while IMM is a hardware memory
model. Subsequently, the verification results produced by <font style="font-variant: small-caps">GenMC</font>
for the two models should be interpreted somewhat differently.

What this means in practice is that, when verifying programs under
RC11, the input file is assumed to be the very source code the user
wrote. A successful verification result under RC11 holds all the
way down to the actual executable, due to the guarantees provided
by RC11 (Lahav et al. 2017).

On the other hand, when verifying programs under IMM, the input file
is assumed to be the assembly code run by the processor (or, more
precisely, a program in IMM's intermediate language).  And while
<font style="font-variant: small-caps">GenMC</font> allows the input file to be a C file (as in the case of
RC11), it assumes that this C file corresponds to an assembly file
that is the result of the compilation of some program in IMM's
language. In other words, program correctness is not preserved across
compilation for IMM inputs.


<a id="org3d3b7e5"></a>

## Race Detection and Memory Errors

For memory models that define the notion of a race, <font style="font-variant: small-caps">GenMC</font> will
report executions containing races erroneous. For instance, under
RC11, the following program is racy, as there is no happens-before
between the write of $x$ in the first thread and the non-atomic
read of $x$ in the second thread (even though the latter causally
depends on the former).

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

Additionally, for all memory models, <font style="font-variant: small-caps">GenMC</font> detects some memory
races like accessing memory that has been already freed, accessing
dynamic memory that has not been allocated, or freeing an already
freed chunk of memory.

Race detection can be completely disabled by means of
`-disable-race-detection`, which may yield better performance for
certain programs.


<a id="orgf6357b1"></a>

## Lock-Aware Partial Order Reduction (<font style="font-variant: small-caps">LAPOR</font>)

For programs that employ coarse-grained locking schemes <font style="font-variant: small-caps">LAPOR</font>
(Kokologiannakis, Raad, and Vafeiadis 2019b) might greatly reduce the state space
and thus the verification time.  For instance, consider the following
program where a lock is used (overly conservatively) to read a shared
variable:

    /* file: lapor.c */
    #include <stdlib.h>
    #include <pthread.h>
    #include <stdatomic.h>
    #include <stdbool.h>
    #include <assert.h>

    #ifndef N
    # define N 2
    #endif

    pthread_mutex_t l;
    int x;

    void *thread_n(void *unused)
    {
            pthread_mutex_lock(&l);
            int r = x;
            pthread_mutex_unlock(&l);
            return NULL;
    }

    int main()
    {
            pthread_t t[N];

            for (int i = 0; i < N; i++) {
                    if (pthread_create(&t[i], NULL, thread_n, NULL))
                            abort();
            }

            return 0;
    }

Running <font style="font-variant: small-caps">GenMC</font> on the program above results in the following outcome:

    Number of complete executions explored: 2
    Total wall-clock time: 0.02s

As expected, as the value of $N$ increases, the executions of the
program also increase in an exponential manner.

However, if we run <font style="font-variant: small-caps">GenMC</font> with `-lapor` on the same program, we get the
following output:

    Number of complete executions explored: 1
    Total wall-clock time: 0.02s

<font style="font-variant: small-caps">LAPOR</font> leverages the fact that the contents of the critical
sections of the threads commute (i.e., the order in which the critical
sections are executed does not matter), and only explores 1 execution
for all values of $N$.

We note that for programs where no further reduction in the
state space is possible, <font style="font-variant: small-caps">LAPOR</font> can be cause a polynomial
slowdown.


<a id="org20cc1f4"></a>

## Barrier-Aware Model Checking (<font style="font-variant: small-caps">BAM</font>)

<font style="font-variant: small-caps">GenMC</font> v0.6 comes with built-in support for `pthread_barrier_t`
functions (see Section [5](#org801c94c)) via <font style="font-variant: small-caps">BAM</font> (Kokologiannakis and Vafeiadis 2021).
As an example of <font style="font-variant: small-caps">BAM</font> in action, consider the following program:

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

Running <font style="font-variant: small-caps">GenMC</font> on the program above results in the following output:

    Number of complete executions explored: 2
    Total wall-clock time: 0.01s

As can be seen, <font style="font-variant: small-caps">GenMC</font> treats `barrier_wait` calls as no-ops,
and they do not lead to any additional explorations. (The two executions
explored correspond to the possible ways in which `x` can be incremented).

However, if we disable <font style="font-variant: small-caps">BAM</font> by means of the `-disable-bam` switch,
get get the following output:

    Number of complete executions explored: 4
    Number of blocked executions seen: 4
    Total wall-clock time: 0.01s

Note that while <font style="font-variant: small-caps">BAM</font> can lead to the exploration of exponentially
fewer executions, it can only be used if the result of the `barrier_wait`
is not used. If it is, then using `-disable-bam` is necessary,
as <font style="font-variant: small-caps">GenMC</font> currently does not enforce this limitation.


<a id="orga4022a7"></a>

## Symmetry Reduction

<font style="font-variant: small-caps">GenMC</font> also employs an experimental symmetry reduction mechanism.
While <font style="font-variant: small-caps">GenMC</font>'s symmetry reduction does not guarantee optimality,
i.e., might explore more executions than what an ideal symmetry
reduction algorithm would (although never more than what the enabled
partitioning dictates), it is still beneficial to use when threads
use the same code.

For instance, if `-sr` is used in the following program, <font style="font-variant: small-caps">GenMC</font>
explores only one execution instead of 6.

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

In order for symmetry reduction to actually take place, the spawned
threads need to share exactly the same code, have exactly the same
arguments, and also there must not be any memory access (at the
LLVM-IR level) between the spawn instructions.


<a id="org8879883"></a>

## Checking Liveness <a id="org6371307"></a>

<font style="font-variant: small-caps">GenMC</font> can also check for liveness violations in programs with
spinloops. Consider the following simple program:

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

Since there are no writes to $x$, the loop in `thread_1` above
will never terminate. Indeed, running <font style="font-variant: small-caps">GenMC</font> with
 `-check-liveness` produces a relevant error report:

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

The `-check-liveness` switch will automatically check for liveness
violations in all loops that have been captured by the spin-assume
transformation (see [4](#org53d284e)).


<a id="orgf475797"></a>

## System Calls and Persistency Checks (<font style="font-variant: small-caps">Persevere</font>) <a id="org152be60"></a>

Since v0.5, <font style="font-variant: small-caps">GenMC</font> supports the verification programs involving
system calls for file manipulation like `read()` and `write()`.  In
addition, using <font style="font-variant: small-caps">Persevere</font> (Kokologiannakis et al. 2021),
<font style="font-variant: small-caps">GenMC</font> can verify persistency properties of such programs. Below
we discuss some details that are important when it comes to verifying
programs that involve file manipulation.


<a id="orgc7d221d"></a>

### Consistency of File-Manipulating Programs

As a first example consider the program below, where a file
`"foo.txt"` is first populated by `main`, and then concurrently
read and written by two threads at offset 0:

    /* file: file-rw.c */
    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <stdatomic.h>
    #include <pthread.h>
    #include <assert.h>

    void *thread_1(void *fdp)
    {
            int fd = *((int *) fdp);
            char buf[8];

            buf[0] = buf[1] = 1;
            int nw = pwrite(fd, buf, 2, 0);
            return NULL;
    }

    void *thread_2(void *fdp)
    {
            int fd = *((int *) fdp);
            char buf[8];

            int nr = pread(fd, buf, 2, 0);
            if (nr == 2)
                    assert((buf[0] == 0 && buf[1] == 0) || (buf[0] == 1 && buf[1] == 1));
            return NULL;
    }

    int main()
    {
            pthread_t t1, t2;
            char buf[8];

            int fd = open("foo.txt", O_CREAT|O_RDWR, 0);

            buf[0] = buf[1] = 0;
            int nw = write(fd, buf, 2);
            assert(nw == 2);

            if (pthread_create(&t1, NULL, thread_1, &fd))
                    abort();
            if (pthread_create(&t2, NULL, thread_2, &fd))
                    abort();

            if (pthread_join(t1, NULL))
                    abort();
            if (pthread_join(t2, NULL))
                    abort();

            return 0;
    }

One property we might be interested in in the above program is whether
the reading thread can see any other (intermediate) state for the file
apart from `00` and `11`. Indeed, as can be seen below, running <font style="font-variant: small-caps">GenMC</font>
on the program above produces an example where the assertion is violated.

    Error detected: Safety violation!
    [...]
    Assertion violation: (buf[0] == 0 && buf[1] == 0) || (buf[0] == 1 && buf[1] == 1)
    Number of complete executions explored: 1
    Total wall-clock time: 0.03s

Apart from safety violations like in this case, <font style="font-variant: small-caps">GenMC</font> will also
report system call failures as errors (e.g., trying to write to a file
that has been opened with `O_RDONLY`). This behavior can be disabled
with `-disable-stop-on-system-error`, which will make <font style="font-variant: small-caps">GenMC</font> report
such errors through `errno`.

When including headers like `stdio.h` or `unistd.h`, <font style="font-variant: small-caps">GenMC</font> intercepts
calls to `open()`, `read()`, `write()`, and other system calls defined
in these header files, and models their behavior. Note that these header
files are also part of <font style="font-variant: small-caps">GenMC</font> so, in general, only the functionality
described in Section [5](#org801c94c) from said header files can be used in programs.

Note that only constant (static) strings can be used as filenames when
using system calls. The filenames need not exist as regular files in
the user's system, as the effects of these system calls are modeled,
and not actually executed. Thus, it is in general preferable if the
contents of the manipulated files maintain a small size across
executions.


<a id="org1ddc7c7"></a>

### Persistency of File-Manipulating Programs

In addition to checking whether safety properties of file-manipulating
programs with regards to consistency are satisfied (as described
above), <font style="font-variant: small-caps">GenMC</font> can also check whether some safety property with
regards to persistency (under `ext4`) is satisfied.  This is achieved
through <font style="font-variant: small-caps">Persevere</font>, which can be enabled with `-persevere`.

For example, let us consider the program below and suppose we want to
check whether, after a crash, it is possible to observe only a part of
an append to a file:

    /* file: pers.c */
    #include <stdio.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <stdatomic.h>
    #include <pthread.h>
    #include <assert.h>
    #include <genmc.h>

    #include <fcntl.h>
    #include <sys/stat.h>

    void __VERIFIER_recovery_routine(void)
    {
            char buf[8];
            buf[0] = 0;
            buf[1] = 0;

            int fd = open("foo.txt", O_RDONLY, 0666);
            assert(fd != -1);

            /* Is is possible to read something other than {2,2} ? */
            int nr = pread(fd, buf, 2, 3);
            if (nr == 2)
                    assert(buf[0] == 2 && buf[1] == 2);
            return;
    }

    int main()
    {
            char buf[8];

            buf[0] = 1;
            buf[1] = 1;
            buf[2] = 1;

            int fd = open("foo.txt", O_CREAT|O_TRUNC|O_RDWR, S_IRWXU);
            write(fd, buf, 3);

            __VERIFIER_pbarrier();

            write(fd, buf + 3, 2);

            close(fd);

            return 0;
    }

In the program above, the `__VERIFIER_pbarrier()` call ensures that
all the file operations before it will be considered as "persisted"
(i.e., having reached disk) in this program. The function
`__VERIFIER_recovery_routine()` is automatically called by <font style="font-variant: small-caps">GenMC</font>
and contains the code to be run by the recovery routine, in order to
observe the post-crash disk state.

In this case, by issuing `genmc -persevere pers.c` we observe that
partly observing the append is indeed possible under `ext4`, as can
be seen below.

    Error detected: Recovery error!
    [...]
    Assertion violation: buf[0] == 2 && buf[1] == 2
    Number of complete executions explored: 2
    Total wall-clock time: 0.08s

For this program in particular, this property is violated due to the
default block size (which is 2 bytes), and the nature of appends in
the default data ordering mode of `ext4` (`data=ordered`).

In general, such parameters of `ext4` can be tuned via the
`--block-size` and `--journal-data` switches (see Section [4](#org53d284e) and
`genmc -help` for more information).  <font style="font-variant: small-caps">GenMC</font> currently assumes a
sector size of 1 byte.


<a id="org2b0335c"></a>

# Command-line Options <a id="org53d284e"></a>

A full list of the available command-line options can by viewed
by issuing `genmc -help`. Below we describe the ones that
are most useful when verifying user programs.

-   **`-rc11`:** Perform the exploration under the RC11 memory model (default)
-   **`-imm`:** Perform the exploration under the IMM memory model
-   **`-lkmm`:** Perform the exploration under the LKMM memory model (experimental)
-   **`-wb`:** Perform the exploration based on the <span style="color: #808080; font-family: monospace">po</span><span style="color: #00ff00; font-family: monospace">rf</span>
    equivalence partitioning.
-   **`-mo`:** Perform the exploration based on the <span style="color: #808080; font-family: monospace">po</span> $\cup$ <span style="color: #00ff00; font-family: monospace">rf</span> $\cup$ <span style="color: #ffa500; font-family: monospace">mo</span>
    equivalence partitioning (default).
-   **`-lapor`:** Enable Lock-Aware Partial Order Reduction (<font style="font-variant: small-caps">LAPOR</font>)
-   **`-disable-bam`:** Disables Barrier-Aware Model-checking (<font style="font-variant: small-caps">BAM</font>)
-   **`-check-liveness`:** Check for liveness violations in spinloops
-   **`-persevere`:** Enable `ext4` persistency checks (<font style="font-variant: small-caps">Persevere</font>)
-   **`-unroll=<N>`:** All loops will be executed at most $N$ times.
-   **`-dump-error-graph=<file>`:** Outputs an erroneous graph to file
    `<file>`.
-   **`-print-error-trace`:** Outputs a sequence of source-code instructions
    that lead to an error.
-   **`-disable-race-detection`:** Disables race detection for non-atomic
    accesses.
-   **`-program-entry-function=<fun_name>`:** Uses function `<fun_name>`
    as the program's entry point, instead of `main()`.
-   **`-disable-spin-assume`:** Disables the transformation of spin loops to
    `assume()` statements.


<a id="org7e9d263"></a>

# Supported APIs <a id="org801c94c"></a>

Apart from C11 API (defined in `stdatomic.h`) and the `assert()`
function used to define safety specifications, below we list supported
functions from different libraries.


<a id="org3ec10dd"></a>

## Supported `stdio`, `unistd` and `fcntl` API

The following functions are supported for I/O:

-   **`int printf(const char *, ...)`:**

-   **`int open (const char *, int , mode_t)`:**

-   **`int creat (const char *, mode_t)`:**

-   **`off_t lseek (int, off_t, int)`:**

-   **`int close (int)`:**

-   **`ssize_t read (int, void *, size_t)`:**

-   **`ssize_t write (int, const void *, size_t)`:**

-   **`ssize_t pread (int, void *, size_t, off_t)`:**

-   **`ssize_t pwrite (int, const void *, size_t, off_t)`:**

-   **`int truncate (const char *, off_t)`:**

-   **`int link (const char *, const char *)`:**

-   **`int unlink (const char *)`:**

-   **`int rename (const char *, const char *)`:**

-   **`int fsync (int)`:**

-   **`void sync (void)`:**

Note that the functions above are modeled and not actually executed,
as described in Section [3.7](#org152be60).


<a id="org55d405b"></a>

## Supported `stdlib` API

The following functions are supported from `stdlib.h`:

-   **`void abort(void)`:**

-   **`int abs(int)`:**

-   **`int atoi(const char *)`:**

-   **`void free(void *)`:**

-   **`void *malloc(size_t)`:**

-   **`void *aligned_alloc(size_t, size_t)`:**


<a id="orgac7c1f9"></a>

## Supported `pthread` API

The following functions are supported from `pthread.h`:

-   **`int pthread_create (pthread_t *, const pthread_attr_t *, void *(*) (void *), void *)`:**

-   **`int pthread_join (pthread_t, void **)`:**

-   **`pthread_t pthread_self (void)`:**

-   **`void pthread_exit (void *)`:**

-   **`int pthread_mutex_init (pthread_mutex_t *, const pthread_mutexattr_t *)`:**

-   **`int pthread_mutex_lock (pthread_mutex_t *)`:**

-   **`int pthread_mutex_trylock (pthread_mutex_t *)`:**

-   **`int pthread_mutex_unlock (pthread_mutex_t *)`:**

-   **`int pthread_mutex_destroy (pthread_mutex_t *)`:**

-   **`int pthread_barrier_init (pthread_barrier_t *, const pthread_barrierattr_t *, unsigned)`:**

-   **`int pthread_barrier_wait (pthread_barrier_t *)`:**

-   **`int pthread_barrier_destroy (pthread_barrier_t *)`:**


<a id="org09067a4"></a>

## Supported SV-COMP (SV-COMP 2019) API

The following functions from the ones defined in SV-COMP (SV-COMP 2019) are supported:

-   **`void __VERIFIER_assume(int)`:**

-   **`int __VERIFIER_nondet_int(void)`:**

Note that, since <font style="font-variant: small-caps">GenMC</font> is a stateless model checker, `__VERIFIER_nondet_int()`
only "simulates" data non-determism, and does actually provide support for it.
More specifically, the sequence of numbers it produces for each thread, remains
the same across different executions.


<a id="org77d7590"></a>

# Contact <a id="orgf2d0f34"></a>

For feedback, questions, and bug reports please send an e-mail to
[michalis@mpi-sws.org](mailto:michalis@mpi-sws.org).

Alglave, Jade, Luc Maranget, Paul E. McKenney, Andrea Parri, and Alan Stern. 2018. “Frightening Small Children and Disconcerting Grown-Ups: Concurrency in the Linux Kernel.” In ASPLOS 2018, 405–18. Williamsburg, VA, USA: ACM. <10.1145/3173162.3177156>.

Kokologiannakis, Michalis, and Viktor Vafeiadis. 2021. “BAM: Efficient Model Checking for Barriers.” In NETYS 2021. LNCS. Springer. <10.1007/978-3-030-91014-3_16>.

Kokologiannakis, Michalis, Ilya Kaysin, Azalea Raad, and Viktor Vafeiadis. 2021. “PerSeVerE: Persistency Semantics for Verification under Ext4.” Proc. ACM Program. Lang. 5 (POPL). New York, NY, USA: ACM. <10.1145/3434324>.

Kokologiannakis, Michalis, Azalea Raad, and Viktor Vafeiadis. 2019a. “Model Checking for Weakly Consistent Libraries.” In PLDI 2019. New York, NY, USA: ACM. <10.1145/3314221.3314609>.

———. 2019b. “Effective Lock Handling in Stateless Model Checking.” Proc. ACM Program. Lang. 3 (OOPSLA). New York, NY, USA: ACM. <10.1145/3360599>.

Kokologiannakis, Michalis, Xiaowei Ren, and Viktor Vafeiadis. 2021. “Dynamic Partial Order Reductions for Spinloops.” In FMCAD 2021, 163–72. IEEE. <10.34727/2021/isbn.978-3-85448-046-4\_25>.

Lahav, Ori, Viktor Vafeiadis, Jeehoon Kang, Chung-Kil Hur, and Derek Dreyer. 2017. “Repairing Sequential Consistency in C/C++11.” In PLDI 2017, 618–32. Barcelona, Spain: ACM. <10.1145/3062341.3062352>.

Podkopaev, Anton, Ori Lahav, and Viktor Vafeiadis. 2019. “Bridging the Gap between Programming Languages and Hardware Weak Memory Models.” Proc. ACM Program. Lang. 3 (POPL). New York, NY, USA: ACM: 69:1–69:31. <10.1145/3290382>.

SV-COMP. 2019. “Competition on Software Verification (SV-COMP).” <https://sv-comp.sosy-lab.org/2019/>.
