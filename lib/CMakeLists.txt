# Turn off automoc/autorcc because Qt isn't used in these third party libs.
# It removes the following dev warning:
#   AUTOGEN: No valid Qt version found for target capstone-static.  AUTOMOC and AUTORCC disabled.
set(CMAKE_AUTOMOC OFF)
set(CMAKE_AUTORCC OFF)

add_subdirectory(libiberty)

set(CAPSTONE_BUILD_STATIC ON CACHE BOOL "")
set(CAPSTONE_BUILD_SHARED OFF CACHE BOOL "")
set(CAPSTONE_BUILD_TESTS OFF CACHE BOOL "")
add_subdirectory(capstone EXCLUDE_FROM_ALL)

# Disable warnings that are not part of this project's scope.
disable_all_warnings(capstone-static)
