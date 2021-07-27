# Changelog

Notable changes to GenMC will be documented in this file. Note
that this repository is only updated whenever a new version of GenMC
is released.

## [Unreleased]

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
