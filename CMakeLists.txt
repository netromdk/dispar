cmake_minimum_required(VERSION 3.0)
project(bmod)

set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

include(platform)
include(target)

#add_subdirectory(examples)

#include(CTest)
#enable_testing()
#add_subdirectory(tests)

# Add a target to generate API documentation with Doxygen.
# find_package(Doxygen)
# if (DOXYGEN_FOUND)
#   configure_file(
#     ${PROJECT_SOURCE_DIR}/doc/doxyfile.in
#     ${CMAKE_CURRENT_BINARY_DIR}/doxyfile @ONLY
#     )
#   add_custom_target(
#     doc ${DOXYGEN_EXECUTABLE}
#     ${CMAKE_CURRENT_BINARY_DIR}/doxyfile
#     WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#     COMMENT "Generating API documentation with Doxygen"
#     VERBATIM
#     )
# endif()

add_subdirectory(src)

message(STATUS "--------------------------------------------------")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "--------------------------------------------------")
