# GenMC: A Generic Model Checker for Concurrent Programs

GenMC is a stateless model checker for concurrent programs written under the
SC [\[Lamport 1979\]](references.md#lamport1979sc),
TSO [\[Owens et al. 2009\]](references.md#owens2009x86-tso),
RA [\[Lahav et al. 2016\]](references.md#lahav2016taming),
RC11 [\[Lahav et al. 2017\]](references.md#lahav2017repairing),
and IMM [\[Podkopaev et al. 2019\]](references.md#podkopaev2019imm) memory models.
It primarily targets C/C++ and Rust programs, verifying safety properties by analyzing
their concurrency patterns (e.g., usage of `C11` atomics or Rust's `std::sync`).
It employs an effective dynamic partial order reduction technique
\[[Kokologiannakis et al. 2019](references.md#kokologiannakis2019genmc), [Kokologiannakis et al. 2022](references.md#kokologiannakis2022trust)\] that
is sound, complete, and optimal.

GenMC operates at the level of LLVM Intermediate Representation (LLVM-IR).
It uses `clang` to translate C/C++ programs and `rustc` to translate Rust programs
into LLVM-IR. This intermediate approach allows GenMC to be language-agnostic regarding
its core verification logic, though it relies on language-specific frontends for
translation.

GenMC should compile on Linux and MacOS provided that the relevant dependencies are
installed (see [README.md](../../README.md)).

## Table of Contents

- [Basic Usage](usage.md)
- [Tool Features](features.md)
- [Command-line Options](cli.md)
- [Supported APIs](api.md)
- [References](references.md)

## Contact

For feedback, questions, and bug reports please send an e-mail
to [michalis.kokologiannakis@inf.ethz.ch](mailto:michalis.kokologiannakis@inf.ethz.ch).
