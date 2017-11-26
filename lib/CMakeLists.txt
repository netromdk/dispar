add_subdirectory(libiberty)

set(CAPSTONE_BUILD_STATIC ON CACHE BOOL "")
set(CAPSTONE_BUILD_SHARED OFF CACHE BOOL "")
set(CAPSTONE_BUILD_TESTS OFF CACHE BOOL "")
add_subdirectory(capstone EXCLUDE_FROM_ALL)

# Disable warnings that are not part of this project's scope.
disable_all_warnings(capstone-static)
