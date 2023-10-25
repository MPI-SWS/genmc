# Changelog

Notable changes to GenMC will be documented in this file. This
repository is only updated whenever a new version of GenMC is
released.


## [Unreleased]

- Documentation


## [0.10.0] - 2023.10.25
### Added

- Support for SC, TSO, RA memory models
- Symmetry reduction (automatic or via `__VERIFIER_spawn_symmetric`)
- In-place revisiting (from Awamoche [CAV 2023])
- Preemption/Round-Robin bounding under SC [TACAS 2023, FMCAD 2023]
- State-space size estimation before verification begins
- Performance optimizations
- Support for LLVM-16
- Rudimentary doxygen documentation

### Changed

- GenMC now requires a compiler with C++20 support

### Deprecated

- Support for LLVM-{7,8,9}

### Removed

- Support for LKMM, Persevere

### Fixes

- Various bug issues


## [0.9] - 2022.12.26
### Added

- Better C++ code support
- Performance optimizations
- Faster `configure`
- Support for LLVM-{14, 15}

### Changed

- GenMC now requires a C++14-compliant compiler

### Removed

- Support for LLVM-6.0
- Dependency on libclang-dev

### Fixes

- Bug fixes


## [0.8] - 2022.02.22
### Added

- Built-in support for hazard pointers
- Annotations for faster data-structure verification via `-helper` (experimental)
- `-no-unroll` switch to exclude functions from unrolling
- Performance optimizations

### Changed

- GenMC now runs under `-mo` by default
- Enhanced graph and DOT printing

### Removed

- Support for LLVM-{3.8, 4.0}

### Fixes

- Documentation
- Various bug fixes


## [0.7] - 2022.01.13
### Added

- Core algorithm now follows TruSt
- Support for concurrent verification (`-nthreads=<uint>`)
- Support for atomic_{nand,umax,umin,max,min}
- Support for LLVM-{12,13}

### Changed

- LAPOR is temporarily disabled

### Deprecated

- Support for LLVM-{3.8,4.0}

### Removed

- Verification of user-defined libraries

### Fixes

- Bug fixes


## [0.6.1] - 2021.07.27
### Added

- Documentation for for `__VERIFIER_spin_{start,end}`
- More accesses to freed memory are detected
- GenMC warns about execution graphs getting too large

### Changed

- Liveness checks are only allowed under `-mo` now

### Fixes

- Bug fixes


## [0.6] - 2021.05.30
### Added

- Support for LKMM (experimental)
- Native support for `pthread_barrier_t` (via BAM)
- More loops are automatically transformed into `assume` statements
- Support for liveness checks reinstated
- Documentation for new features

### Changed

- Error message for mixed-size accesses

### Fixes

- GenMC's output is now properly directed to stdout
- Proper support for `pthread_mutex_init` and `pthread_mutex_destroy`
- Various bugs


## [0.5.3] - 2020.12.06
### Added

- Support for `aligned_alloc()`

### Fixes

- Fixes for ppo calculation in IMM
- `malloc()` now returns aligned addresses
- Documentation fix and update
- Other minor bugs


## [0.5.2] - 2020.10.08
### Added

- Documentation for Persevere, LAPOR


## [0.5.1] - 2020.10.01
### Added

- Support for LLVM 11

### Removed

- Temporarily removed liveness support for fixing

### Fixes

- Build fixes for LLVM 10


## [0.5] - 2020.09.23
### Added

- Support for some unistd.h system calls
- Support for checking persistency semantics of files under ext4 (experimental)
- Race detection on automatic variables
- Checking for accessing uninitialized memory
- Symmetry reduction (unstable)
- Liveness checks (experimental)
- Support for memcpy() (experimental)
- Support for C++ (experimental)
- Support for <threads.h>
- Support for LLVM-9
- New CHANGELOG.md file to track changes in the project.

### Changed

- Names of various switches

### Removed

- Support for LLVM 3.5

### Fixes

- Various bug fixes
- Test cases under macOS
