# Changelog

All notable changes to GenMC will be documented in this file. Note
that this repository is only updated whenever a new version of GenMC
is released.

## [Unreleased]

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
