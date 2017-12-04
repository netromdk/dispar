# Contributing
Contributions are very welcome for bug fixes particularly but also features that make sense to the project. Any existing issues labeled ["help wanted"](https://github.com/netromdk/dispar/labels/help%20wanted) are free to be pursued. And don't hesitate to open new issues.

Before opening a pull request, please first run [clang format](https://clang.llvm.org/docs/ClangFormat.html) on the code, then make sure that all tests pass and to add new tests where appropriate. See [Testing](README.md#testing).

## C++ Guidelines
* Use `assert()` instead of `Q_ASSERT()` because the former works properly on all platforms, _including MSVC_, and the latter does `exit(1)` instead of `abort()` in MSVC. Remember to include `<cassert>`.
