# Building GenMC

## Build options

 - `GENMC_DEBUG`: this CMake option will enable diagnostics for GenMC (enabled by default for `Debug` builds)
 - `GENMC_WERROR`: treat compiler warnings as errors for GenMC's own code (`genmc/` and `passes/`); default: OFF, enabled in CI
 - `ENABLE_COVERAGE`: enables coverage reporting (default: OFF)
 - `BUILD_DOCS`: builds documentation (default: OFF)
 - `SANITIZE`: comma-separated sanitizers to enable, e.g. `address,undefined` or `thread` (default: empty)
 - `GENMC_TCMALLOC`: link `genmc` against tcmalloc (faster allocator): `AUTO` (default, if found) / `ON` (required) / `OFF`. Needs `libgoogle-perftools-dev`; disabled under `SANITIZE`. Pin `ON`/`OFF` for reproducible builds.

# Testing GenMC

## Running tests

Run some quick tests:
```shell
./scripts/fast-driver.sh
```

Environment variables to affect the testing scripts:
```shell
  GenMC="path/to/genmc/executable"  # Select the GenMC executable to use for the test (default: `${SCRIPT_DIR}/../RelWithDebInfo/bin/genmc`)
  GENMCFLAGS="--flag --another"     # Pass extra flags to any GenMC invocation.
```

Example:
```shell
GenMC="Release/bin/genmc" GENMCFLAGS="--disable-estimation" ./scripts/fast-driver.sh
```

## Building with sanitizers

To build with AddressSanitizer + UBSan:
```shell
cmake -DGENMC_DEBUG=ON -DBUILD_TESTS=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo \
      -DSANITIZE=address,undefined \
      -DCMAKE_PREFIX_PATH=/usr/lib/llvm-19/cmake \
      -B Asan -S .
cmake --build Asan -j$(nproc)
ASAN_OPTIONS=detect_leaks=0 ./Asan/bin/unit_tests
GenMC=./Asan/bin/genmc ASAN_OPTIONS=detect_leaks=0 ./scripts/fast-driver.sh
```

`ASAN_OPTIONS=detect_leaks=0` suppresses leak reports from LLVM's arena allocators, which
intentionally do not free on exit. For ThreadSanitizer use `SANITIZE=thread`.
`address` and `thread` are mutually exclusive.

# Modifying GenMC

## Pull requests

Please submit PRs as series of atomic, easily reversible commits.
Code must be formatted with `clang-format` and free of `clang-tidy` warnings (`scripts/lint.sh`).

## Code conventions

### Comments

Please use `/*` and `*/` as comment delimiters.

Initially, these were used to separate our modifications from the
original code in LLVM code (which uses `//`), but now it's just a
convention.

### Classes

- Declaration order within a class (public -> protected -> private):
  1. Types and type aliases
  2. Static constants/functions (e.g., factories)
  3. Ctors, assignment operators and dtor
  4. All other functions (ending with operators and friends)
  5. Data members (static -> nonstatic)

### Naming

- CamelCase for everything apart from things mimicking STL
  functionality (LLVM utilities remain unmodified)
- Names start with a lowercase letter
- Member variables should end with an underscore (_)

### Error and assertion macros

`genmc/genmc/Support/Error.hpp` provides three macros for invariant checking.
Their behavior can be tuned with `GENMC_DEBUG`.

| Macro | Debug | Release |
|-------|-------|---------|
| `VERIFY(cond[, msg])` | Crashes with message | Crashes with message |
| `ASSERT(cond[, msg])` | Crashes with message | No-op |
| `UNREACHABLE([msg])` | Crashes with message | `__builtin_trap()` (SIGILL) |

## Adding new label types

To add a new label, follow these steps:

1. Add a number and a name for the new label in `genmc/genmc/Execution/EventLabel.def`
2. Add a class for the new label in `genmc/genmc/Execution/EventLabel.hpp`.
   Macros are provided for dummy labels and standard subclasses
3. Add a case for the new label in `genmc/genmc/Execution/LabelVisitor.hpp`
4. Define how the label should be printed in `genmc/genmc/Execution/EventLabel.cpp`
   (optionally: LabelPrinterBase too)
5. Create a handler for the new label in `genmc/genmc/Verification/GenMCDriver.{hpp,cpp}`
   (if necessary), and at DriverHandlerDispatcher
6. In case a new LLVM-IR function leads to the creation of
   the new label:
   - Add a number and a name for the internal function in
   `passes/passes/InternalFunction.def`.
   - Define a function that dispatches the driver at
   `lli/Runtime/Execution.cpp`

*Note:* the procedure above describes the bare minimum, and works well
for dummy labels or subclasses of existing labels. If e.g., the new
label has attributes like location then extra changes might be
required to ensure that iterators, etc still work.
