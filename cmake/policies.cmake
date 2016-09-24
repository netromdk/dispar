# Only interpret if() arguments as variables or keywords when unquoted.
cmake_policy(SET CMP0054 NEW)

# Target names should not be reserved and should match a validity pattern. Target names reserved by
# one or more CMake generators are not allowed. Among others these include "all", "help" and
# "test".
cmake_policy(SET CMP0037 NEW)