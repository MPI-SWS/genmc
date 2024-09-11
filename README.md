GenMC: Generic Model Checking for C Programs
=====

GenMC is a stateless model checker for C programs that works on the
level of LLVM Intermediate Representation.

This repository mirrors an internal repository and is only updated periodically.
For changes between different versions please refer to the CHANGELOG.

Author: Michalis Kokologiannakis.

* [Getting GenMC](#getting-genmc)
* [Usage](#usage)
* [Troubleshooting](#troubleshooting)
* [License](#license)
* [Contact](#contact)

<a name="getting-genmc">Getting GenMC</a>
-----------------------------------------

### Using Docker

To pull a container containing GenMC from [Docker Hub](https://hub.docker.com)
please issue the following command:

		docker pull genmc/genmc

### Building from source

#### Dependencies

You will need a C++20-compliant compiler and an LLVM installation.
The LLVM versions currently supported are:
10.0.1, 11.0.0, 12.0.1, 13.0.0, 14.0.0, 15.0.0, 16.0.0 (deprecated:
7.0.1, 8.0.1, 9.0.1).

##### GNU/Linux

In order to use the tool on a Debian-based installation, you need the
following packages:

		autoconf  automake  clang  llvm  llvm-dev  libffi-dev
		zlib1g-dev libedit-dev

##### Max OS X

Using `brew`, the following packages are necessary:

		autoconf automake llvm libffi

#### Installing

##### GNU/Linux

For a default build issue:

		autoreconf --install
		./configure
		make

This will leave the `genmc` executable in the build directory.
You can either run it from there (as in the examples below), or issue
`make install`.

Alternatively, the following following command will build the `genmc`
executable in parallel and will also run a subset of all the tests
that come with the system to see if the system was built correctly or
not:

		make -j check

##### Mac OS X

For a default build issue:

		autoreconf --install
		./configure AR=llvm-ar
		make

<a name="usage">Usage</a>
-------------------------

* To see a list of available options run:

		./genmc --help

* To run a particular testcase run:

		./genmc [options] <file>

* For more detailed usage examples please refer to the [manual](doc/manual.md).


<a name="troubleshooting">Troubleshooting</a>
---------------------------------------------

* Undefined references to symbols that involve types `std::__cxx11` during linking:

	This probably indicates that you are using an old version of LLVM with a new
	version of libstdc++. Configuring with the following flags should fix the problem:

			CXXFLAGS="-D_GLIBCXX_USE_CXX11_ABI=0" ./configure --with-llvm=LLVM_PATH

* Linking problems under Arch Linux:

    Arch Linux does not provide the `libclang*.a` files required for linking
	against `clang` libraries. In order to for linking to succeed, please
	change the last line in `src/Makefile.am` to the following:

			genmc_LDADD = libgenmc.a -lclang-cpp


<a name="license">License</a>
-----------------------------

GenMC (with the exception of some files, see [Exceptions](#exceptions))
is distributed under the GPL, version 3 or (at your option) later.
Please see the COPYING file for details on GPLv3.

### <a name="exceptions">Exceptions</a>

Part of the code in the files listed below are originating from
the [LLVM Compiler Framework](https://llvm.org), version 3.5.
These parts are licensed under the University of Illinois/NCSA
Open Source License as well as under GPLv3. Please see the LLVMLICENSE
file for details on the University of Illinois/NCSA Open Source License.

		src/Interpreter.h
		src/Interpreter.cpp
		src/Execution.cpp
		src/ExternalFunctions.cpp

Additionally, the files within the `include` directory are licensed
under their own licenses.

Please see the respective header for more information on a specific
file's license.


<a name="contact">Contact</a>
------------------------

For feedback, questions, and bug reports please send an e-mail to
`michalis AT mpi-sws DOT org`.
