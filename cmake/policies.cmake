# Only interpret if() arguments as variables or keywords when unquoted.
if (CMP0054)
  cmake_policy(SET CMP0054 NEW)
endif()

# Target names should not be reserved and should match a validity pattern. Target names reserved by
# one or more CMake generators are not allowed. Among others these include "all", "help" and
# "test".
if (CMP0037)
  cmake_policy(SET CMP0037 NEW)
endif()
