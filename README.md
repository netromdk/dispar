[![Build Status](https://travis-ci.org/netromdk/dispar.svg?branch=master)](https://travis-ci.org/netromdk/dispar)

# dispar
Dispar is short for "[Dis]assemling binary [Par]ser" written in C++14. The whole concept of the project is to load binaries, like executables, libraries, core dumps etc., and do analysis of their structure and data; most notably their strings, symbols, and functions. Currently, it supports only 32+64 bit [Mach-O](https://en.wikipedia.org/wiki/Mach-O) binaries (including [universal binaries](https://en.wikipedia.org/wiki/Universal_binary)) but there are plans for supporting [ELF](https://en.wikipedia.org/wiki/Executable_and_Linkable_Format) and [PE/PE+](https://en.wikipedia.org/wiki/Portable_Executable) later on.

# Table of Contents
* [Dependencies](#dependencies)
  * [External](#external)
  * [Internal](#internal)
* [Building The Program](#building-the-program)
  * [CCache](#ccache)
* [Testing](#testing)
* [Debugging](#debugging)
* [Code Coverage](#code-coverage)
* [Static Analysis](#static-analysis)
* [Troubleshooting](#troubleshooting)
* [Contributing](#contributing)

# Dependencies
There are both external and internal libraries and tools required to build and run this program.

## External
* CMake 3.1+
* Qt 5+
  * QtCore
  * QtGui
  * QtWidgets
  * Arch-specific platform plugin: libqcocoa.dylib, qwindows.dll, or libqxcb.so
* Clang, llvm-profdata, and llvm-cov 4+ (for code coverage)
* Clang, scan-build 4+ (for static analysis)

## Internal
Located in the [lib](lib) folder:
* [Capstone](https://github.com/aquynh/capstone) (disassembler)
* [libiberty](https://github.com/gcc-mirror/gcc/tree/master/libiberty) (demangling)

# Building The Program
```
% mkdir build
% cd build
% cmake ..
% make
```

This will create the dispar executable in "./bin/".

## CCache
If you find yourself building and rebuilding often then ccache can speed up both compilation and linking significantly. For this project I get ~15X speed up.
```
% cmake -DUSE_CCACHE=ON .
% make
```

Note that you will get the speed up after compiling the second time because the first time ccache hasn't cached the compiled objects yet.

**NOTE: In the following all cmake invocations assume they're being run from the "./build" folder!**

# Testing
A suite of software tests can be run like this:
```
% cmake -DBUILD_TESTS=ON .
% make
% ./bin/tests
```

# Debugging
When debugging a bug or a weird scenario, it is often a good idea to use Clang's [Address Sanitizer](https://clang.llvm.org/docs/AddressSanitizer.html) that is great at detecting memory errors:

```
% cmake -DADDRESS_SANITIZER=ON .
% make
% # Run program or tests from terminal to get stdout/stderr output
```
Note that enabling `ADDRESS_SANITIZER` will automatically switch to debug build type.

# Code Coverage
Code coverage via tests can be run via the following (*Clang 4+ is required*):
```
% cmake -DCODE_COVERAGE=ON .
% make codecov
```

It will generate a textual output and an HTML report located in "./report.dir/index.html".

When your binaries are named differently, cmake can be configured in the following way instead:
```
% CC=clang-mp-5.0 \
  CXX=clang++-mp-5.0 \
  cmake -DCODE_COVERAGE=ON -DLLVM_PROFDATA=llvm-profdata-mp-5.0 -DLLVM_COV=llvm-cov-mp-5.0 ..
```

# Static Analysis
Do static analysis and present report in browser (*Clang 4+ is required*):
```
% scan-build cmake -DSTATIC_ANALYZER=ON ..
% make sa
```

When your binaries are named differently, cmake can be configured in the following way instead:
```
% CC=clang-mp-5.0 \
  CXX=clang++-mp-5.0 \
  scan-build-mp-5.0 \
  cmake -DSTATIC_ANALYZER=ON -DSCAN_BUILD=scan-build-mp-5.0 ..
```

# Troubleshooting
If for any reason a specific macOS SDK is required, tell cmake this way:
```
% cmake -DFORCE_MAC_SDK=X.Y .
```
Where "X.Y" is the version of the SDK, like "10.8".

# Contributing
See [CONTRIBUTING.md](CONTRIBUTING.md).
