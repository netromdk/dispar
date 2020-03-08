# Turn off automoc/autorcc because Qt isn't used in these third party libs.
# It removes the following dev warning:
#   AUTOGEN: No valid Qt version found for target capstone-static.  AUTOMOC and AUTORCC disabled.
set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTORCC OFF)

add_subdirectory(libiberty)

# Build capstone as static library, static runtime (MSVC), and with default allocator.
set(CAPSTONE_BUILD_STATIC ON CACHE BOOL "")
set(CAPSTONE_BUILD_STATIC_RUNTIME ON CACHE BOOL "")
set(CAPSTONE_USE_DEFAULT_ALLOC ON CACHE BOOL "")

# Don't disable AT&T syntax and don't reduce x86 instruction sets.
set(CAPSTONE_X86_ATT_DISABLE OFF CACHE BOOL "")
set(CAPSTONE_X86_REDUCE OFF CACHE BOOL "")

# Turn off anything unnecessary.
set(CAPSTONE_BUILD_SHARED OFF CACHE BOOL "")
set(CAPSTONE_BUILD_TESTS OFF CACHE BOOL "")
set(CAPSTONE_BUILD_CSTOOL OFF CACHE BOOL "")
set(CAPSTONE_BUILD_DIET OFF CACHE BOOL "")
set(CAPSTONE_OSXKERNEL_SUPPORT OFF CACHE BOOL "")

add_subdirectory(capstone EXCLUDE_FROM_ALL)

# Disable warnings that are not part of this project's scope.
disable_all_warnings(capstone-static)
