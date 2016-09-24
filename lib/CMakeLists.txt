add_subdirectory(libiberty)

set(CAPSTONE_BUILD_STATIC ON)
set(CAPSTONE_BUILD_SHARED OFF)
set(CAPSTONE_BUILD_TESTS OFF)
add_subdirectory(capstone)