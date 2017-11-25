# Advanced Topics

# Table of Contents
* [Ninja](#ninja)
* [CCache](#ccache)
* [Debugging](#debugging)
* [Code Coverage](#code-coverage)
* [Static Analysis](#static-analysis)
* [Troubleshooting](#troubleshooting)

# Ninja
Instead of using makefiles, I prefer [ninja](https://ninja-build.org) as a build system because it's faster and handles lots of build dependencies safer and better.

After having installed ninja, make sure to remove and recreate your build folder before doing this:
```
% cmake -G Ninja ..          # in ./build
% ninja
```

Note that you don't have to specify how many concurrent jobs it should do because it'll derive this automatically from the number of CPUs available.

# CCache
If you find yourself building and rebuilding often then [ccache](https://github.com/ccache/ccache) can speed up both compilation and linking significantly. For this project I get ~15X speed up.

The only requirement for the following to work is that the "ccache" executable can be found in `$PATH`:
```
% cmake -DUSE_CCACHE=ON .
% make
```

Note that you will get the speed up after compiling the second time because the first time ccache hasn't cached the compiled objects yet.

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
