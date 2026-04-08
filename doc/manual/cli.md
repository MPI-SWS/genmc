# Command-line Options

A full list of the available command-line options can by viewed by issuing `genmc -help`. Below we describe the ones that are most useful when verifying user programs.

- **`-sc`**: Perform the exploration under the SC memory model
- **`-tso`**: Perform the exploration under the TSO memory model
- **`-ra`**: Perform the exploration under the RA memory model
- **`-rc11`**: Perform the exploration under the RC11 memory model (default)
- **`-imm`**: Perform the exploration under the IMM memory model
- **`-nthreads=<N>`**: Perform verification concurrently (using `N` threads)
- **`-cache-instructions`**: Caches instructions to help execution time (sacrifices memory)
- **`-disable-bam`**: Disables Barrier-Aware Model-checking (BAM)
- **`-check-liveness`**: Check for liveness violations in spinloops
- **`-unroll=<N>`**: All loops will be executed at most `N` times.
- **`-dump-error-graph=<file>`**: Outputs an erroneous graph to file `<file>`.
- **`-print-error-trace`**: Outputs a sequence of source-code instructions that lead to an error.
- **`-disable-race-detection`**: Disables race detection for non-atomic accesses.
- **`-program-entry-function=<fun_name>`**: Uses function `<fun_name>` as the program's entry point, instead of `main()`.
- **`-disable-spin-assume`**: Disables the transformation of spin loops to `assume()` statements.
