# Usage

GenMC accepts LLVM-IR as input, which can be generated from various source languages.
Below we detail how to invoke GenMC for C/C++ and Rust programs.

A generic invocation of GenMC resembles the following:

```sh
genmc [OPTIONS] -- [COMPILER_FLAGS] <file>
```

In the above command, `OPTIONS` include several options that can be passed to
GenMC (see [Command-line Options](cli.md)), and `COMPILER_FLAGS` are the options that one would normally
pass to the compiler (e.g., `clang` or `rustc`). If no such flags exist, the `--` can be omitted.

Note that, in order for GenMC to be able to verify it, `<file>` needs
to meet two requirements: finiteness and data-determinism.  Finiteness
means that all tests need to have finite traces, i.e., no infinite
loops (these need to be bounded; see [Handling Infinite
Loops](#handling-infinite-loops)).  Data-determinism means that the
code under test should be data-deterministic, i.e., not perform
actions like calling `rand()`, performing actions based on user input
or the clock, etc.  In other words, all non-determinism should
originate either from the scheduler or the underlying (weak) memory
model.

As long as these requirements are satisfied, GenMC will detect safety
errors, races on non-atomic variables, as well as some memory errors
(e.g., double-free error). Users can provide safety specifications for
their programs by using `assert()` statements.

## Verifying C/C++ Programs

To verify a C or C++ file, pass it directly to `genmc`. The file should use the `stdatomic.h` and `pthread.h` APIs for concurrency.

### A First Example

Consider the following program, demonstrating the Message-Passing (MP) idiom:

```c
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
```

In order to analyze the code above with GenMC, we can use the following command:

```sh
genmc mp.c
```

with which GenMC will yield the following result:

```sh
Number of complete executions explored: 2
Total wall-clock time: 0.02s
```

GenMC explores two executions: one where `ready` = `data` = 0, and one where `ready` = `data` = 1.

## Verifying Rust Programs

GenMC can verify Rust programs by utilizing `rustc` to emit LLVM-IR. This requires a custom wrapper for Rust’s standard library to support GenMC's specific concurrency primitives.

**Prerequisites:**
1.  GenMC must be compiled with the `ENABLE_RUST` flag set.
2.  A path to the Rust installation must be provided via `CMAKE_PREFIX_PATH` during compilation (see [README](../../README.md)).
3.  GenMC must be compiled against the same LLVM major version used by the target Rust installation (check with `rustc -vV`).

See [Supported APIs](api.md) for the list of available Rust APIs.

### Verifying a Cargo Project

To verify a Cargo project, pass the `Cargo.toml` file as the input file. The project must include the `genmc-std` crate provided by GenMC as a dependency, explicitly aliased as `std`:

```toml
[dependencies]
std = { path = './rust/genmc-std', package = 'genmc-std'}
```

With this configuration, you can use functions like `std::thread::spawn(...)` exactly as you would in standard Rust code, without any changes to your source. Avoid referencing `genmc-std` directly (e.g., do *not* use `genmc-std::thread::spawn`).

### Verifying a Single Rust File

A single Rust file (`*.rs`) can also be passed directly to GenMC. In this mode, GenMC automatically handles the binding of the `genmc-std` crate. You can use `std` functionality as normal.

## Reducing the State Space of a Program With `assume()` Statements

In some programs, we only care about what happens when certain reads
read certain values of interest. That said, by default, GenMC will
explore all possible values for all program loads, possibly leading to
the exploration of an exponential number of executions.

To alleviate this problem, GenMC supports the `__VERIFIER_assume()`
function (similar to the one specified in SV-COMP
[\[SV-COMP 2019\]](references.md#sv-comp)). This function takes an integer
argument (e.g., the value read from a load), and only continues the
execution if the argument is non-zero.

For example, let us consider the MP program from the previous section,
and suppose that we are only interested in verifying the assertion in
cases where the first read of the second thread reads 1. We can use an
`assume()` statement to achieve this, as shown below:

```c
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
```

Note that the `__VERIFIER_assume()` function has to be declared. Alternatively, one can include the `<genmc.h>` header, that contains the declarations for all the special functions that GenMC offers (see [Supported APIs](api.md)).

If we run GenMC on the `mp-assume.c` program above, we get the following output:

```sh
Number of complete executions explored: 1
Number of blocked executions seen: 1
Total wall-clock time: 0.02s
```

As can be seen, GenMC only explored one full execution (the one where
`r` = 1), while the execution where `r` = 0 was blocked because of the
`assume()` statement. Of course, while the usage of `assume()` does
not make any practical difference in this small example, this is not
always the case: generally, using `assume()` might yield an
exponential improvement in GenMC's running time.

Finally, note that, when using GenMC under memory models that track
dependencies (see [Available Memory
Models](features.md#available-memory-models)), an `assume()` statement
will introduce a control dependency in the program code.

## Handling Infinite Loops

As mentioned in the beginning of this section, all programs that GenMC
can handle need to have finite traces. That said, many programs of
interest do not fulfill this requirement, because, for example, they
have some infinite loop. GenMC offers three solutions for such cases.

First, GenMC can automatically perform the "spin-assume"
transformation for a large class of spinloops. Specifically, as long
as a spinloop completes a full iteration with no visible side effects
(e.g., stores to global variables), GenMC will cut the respective
execution. For instance, consider the following simple loop:

```c
int r = 0;
while (!atomic_compare_exchange_strong(&x, &r, 1))
        r = 0;
```

Since this loop has no visible side-effects whenever it completes a
full iteration, GenMC will not explore more than one execution where
the loop fails (the execution where the loop fails will be reported as
a blocked execution). The "spin-assume" transformation has proven to
be very effective for a wide range of loops; for more details on
whether it applies on a specific loop, please see [\[Kokologiannakis et
al. 2021\]](references.md#kokologiannakis2021saver).

Finally, for infinite loops with side effects, we can use the
`-unroll=N` command-line option (see [Command-line
Options](cli.md)). This option bounds all loops so that they are
executed at most `N` times. In this case, any verification guarantees
that GenMC provides hold up to that bound. If you are unsure whether
you should use the `-unroll=N` switch, you can try to verify the
program and check whether GenMC complains about the graph size
(`-warn-on-graph-size=<N>`). If it does, there is a good chance you
need to use the `-unroll=N` switch.

Note that the loop-bounding happens at the LLVM-IR level, which means
that the loops there may not directly correspond to loops in the C
code (depending on the enabled compiled optimizations, etc.).

## Error Reporting

In the previous sections, saw how GenMC verifies the small MP
program. Let us now proceed with an erroneous version of this program,
in order to show how GenMC reports errors to the user.

Consider the following variant of the MP program below, where the
store to `ready` in the first thread is now performed using a relaxed
access:

```c
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
```

This program is buggy since the load from `ready` no longer
synchronizes with the corresponding store, which in turn means that
the load from `data` may also read 0 (the initial value), and not just
42.

Running GenMC on the above program, we get the following outcome:

```sh
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
```

GenMC reports an error and prints some information relevant for
debugging. First, it prints the type of the error, then the execution
graph representing the erroneous execution, and finally the error
message, along with the executions explored so far and the time that
was required.

The graph contains the events of each thread along with some
information about the corresponding source-code instructions. For
example, for write events (e.g., event (1, 1)), the access mode, the
name of the variable accessed, the value written, as well as the
corresponding source-code line are printed. The situation is similar
for reads (e.g., event (2, 1)), but also the position in the graph
from which the read is reading from is printed.

Note that there are many different types of events. However, many of
them are GenMC-related and not of particular interest to users (e.g.,
events labeled with `B`, which correspond to the beginning of a
thread). Thus, GenMC only prints the source-code lines for events that
correspond to actual user instructions, thus helping the debugging
procedure.

Finally, when more information regarding an error are required, two
command-line switches are provided. The `-dump-error-graph=<file>`
switch provides a visual representation of the erroneous execution, as
it will output the reported graph in DOT format in `<file>`, so that
it can be viewed by a PDF viewer. Finally, the `-print-error-trace`
switch will print a sequence of source-code lines leading to the
error. The latter is especially useful for cases where the bug is not
caused by some weak-memory effect but rather from some particular
interleaving (e.g., if all accesses are `memory_order_seq_cst`), and
the write where each read is reading from can be determined simply by
locating the previous write in the same memory location in the
sequence.
