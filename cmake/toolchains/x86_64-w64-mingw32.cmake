# Cross-compile for Windows with MinGW-w64 GCC (e.g. Debian's g++-mingw-w64-x86-64-posix):
#   cmake -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/x86_64-w64-mingw32.cmake \
#         -DBUILD_LLI=OFF -DBUILD_PASSES=OFF -DBUILD_TESTS=ON -B build-mingw -S .
# Only the core library builds this way; lli/ and passes/ need an LLVM built for MinGW
# (MSYS2 ships one, Linux distributions do not).

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(_triple x86_64-w64-mingw32)
# Debian installs the POSIX-threads flavor under a -posix suffix
find_program(CMAKE_C_COMPILER NAMES ${_triple}-gcc-posix ${_triple}-gcc)
find_program(CMAKE_CXX_COMPILER NAMES ${_triple}-g++-posix ${_triple}-g++)

# Headers and libraries live next to the compiler (<prefix>/x86_64-w64-mingw32)
get_filename_component(_bindir "${CMAKE_CXX_COMPILER}" DIRECTORY)
set(CMAKE_FIND_ROOT_PATH "${_bindir}/../${_triple}")
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

# Static executables run under Wine without the MinGW DLLs
set(CMAKE_EXE_LINKER_FLAGS_INIT "-static")

# Run test executables through Wine when available
find_program(_wine NAMES wine64 wine)
if(_wine)
  set(CMAKE_CROSSCOMPILING_EMULATOR "${_wine}")
endif()
