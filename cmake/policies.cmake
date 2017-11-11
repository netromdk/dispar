# Only interpret if() arguments as variables or keywords when unquoted.
if (POLICY CMP0054)
  cmake_policy(SET CMP0054 NEW)
endif()

# Target names should not be reserved and should match a validity pattern. Target names reserved by
# one or more CMake generators are not allowed. Among others these include "all", "help" and
# "test".
if (POLICY CMP0037)
  cmake_policy(SET CMP0037 NEW)
endif()

# Fix behavior of CMAKE_CXX_STANDARD when targeting macOS.
if (POLICY CMP0025)
  cmake_policy(SET CMP0025 NEW)
endif()
