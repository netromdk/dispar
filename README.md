[![Build Status](https://travis-ci.org/netromdk/dispar.svg?branch=master)](https://travis-ci.org/netromdk/dispar)

# dispar
Dispar is short for "[Dis]assemling binary [Par]ser" written in C++14. The whole concept of the project is to load binaries, like executables, libraries, core dumps etc., and do analysis of their structure and data; most notably their strings, symbols, and functions. Currently, it supports only 32+64 bit [Mach-O](https://en.wikipedia.org/wiki/Mach-O) binaries (including [universal binaries](https://en.wikipedia.org/wiki/Universal_binary)) but there are plans for supporting [ELF](https://en.wikipedia.org/wiki/Executable_and_Linkable_Format) and [PE/PE+](https://en.wikipedia.org/wiki/Portable_Executable) later on.

Table of contents
=================

* [Dependencies](#dependencies)
* [Building](#building)
* [Testing](#testing)
* [Contributions](#contributions)

Dependencies
============
There are both external and internal libraries and tools required to build and run this program.

### External
* CMake 3+
* Qt 5+
  * QtCore
  * QtGui
  * QtWidgets
  * Arch-specific platform plugin: libqcocoa.dylib, qwindows.dll, or libqxcb.so

### Internal
Located in the [lib](lib) folder:
* [Capstone](https://github.com/aquynh/capstone)
* [libiberty](https://github.com/gcc-mirror/gcc/tree/master/libiberty)

Building
========
```
% mkdir build
% cd build
% cmake ..
% make
```

This will create the dispar executable in "./bin/".

Testing
=======
A suite of software tests can be run in two ways:
```
% make && make test
```

Or:
```
% make && ./bin/tests
```

Contributions
=============
Contributions are very welcome for bug fixes particularly but also features that make sense to the project. Any existing issues labeled ["help wanted"](https://github.com/netromdk/dispar/labels/help%20wanted) are free to be pursued. And don't hesitate to open new issues.

Before opening a pull request, please first run [clang format](https://clang.llvm.org/docs/ClangFormat.html) on the code, then make sure that all tests pass and to add new tests where appropriate. See [Testing](#testing).
