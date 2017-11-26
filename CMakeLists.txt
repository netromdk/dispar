cmake_minimum_required(VERSION 3.1)
project(dispar)

set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(policies NO_POLICY_SCOPE)

option(BUILD_TESTS "Build test suite" OFF)
option(USE_CCACHE "Use ccache (if found) to speed up compile and link" OFF)
option(CODE_COVERAGE "Instrument for code coverage (clang only!)" OFF)
option(ADDRESS_SANITIZER "Use memory error detector  (clang only and implies debug mode!)" OFF)
option(STATIC_ANALYZER "Do static analysis (clang only and implies debug mode!)" OFF)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Output compile_commands.json (Ninja generator only!)
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}")

include(misc)
include(platform)
include(target)

if (CODE_COVERAGE)
  if (NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    message(FATAL_ERROR "Clang is required to do code coverage!")
  endif()

  if (NOT LLVM_PROFDATA)
    set(LLVM_PROFDATA llvm-profdata)
  endif()

  if (NOT LLVM_COV)
    set(LLVM_COV llvm-cov)
  endif()

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-instr-generate -fcoverage-mapping")
endif()

if (ADDRESS_SANITIZER)
  if (NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    message(FATAL_ERROR "Clang is required to use address sanitizer to detect memory errors!")
  endif()

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O1 -g -fsanitize=address -fno-omit-frame-pointer -fno-optimize-sibling-calls")
  set(CMAKE_BUILD_TYPE Debug)
endif()

if (STATIC_ANALYZER)
  if (NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    message(FATAL_ERROR "Clang is required to do static analysis!")
  endif()

  if (NOT SCAN_BUILD)
    set(SCAN_BUILD scan-build)
  endif()

  set(CMAKE_BUILD_TYPE Debug)
endif()

# Names
set(libName core)
set(binName dispar)
set(testName tests)

if (BUILD_TESTS)
  include(CTest)
  enable_testing()
  add_subdirectory(tests)
endif()

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

add_subdirectory(lib)
add_subdirectory(src)

# Requires llvm/clang v4+!
# Setup: CC=clang-mp-5.0 CXX=clang++-mp-5.0 cmake -G Ninja -DCODE_COVERAGE=ON -DLLVM_PROFDATA=llvm-profdata-mp-5.0 -DLLVM_COV=llvm-cov-mp-5.0 ../../
if (CODE_COVERAGE)
  add_custom_target(
    codecov

    sh ${CMAKE_SOURCE_DIR}/scripts/code_coverage.sh "${CMAKE_CURRENT_BINARY_DIR}" "$<TARGET_FILE:${testName}>" "${LLVM_PROFDATA}" "${LLVM_COV}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Generating test code coverage report"
    DEPENDS ${testName}
    )
endif()

# Setup: scan-build-mp-5.0 cmake -DSTATIC_ANALYZER=ON -DSCAN_BUILD=scan-build-mp-5.0 ../../
if (STATIC_ANALYZER)
  add_custom_target(
    sa

    sh ${CMAKE_SOURCE_DIR}/scripts/static_analysis.sh ${SCAN_BUILD} ${CMAKE_CURRENT_BINARY_DIR} ${Qt5Core_DIR}/../../ ${binName}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "Doing static analysis"
    )
endif()

# cppcheck can be cloned and compiled: git clone git@github.com:danmar/cppcheck.git
add_custom_target(
  cppcheck

  cppcheck --enable=all --project=${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json --xml 2> cppcheck.xml && python ${CMAKE_SOURCE_DIR}/scripts/cppcheck-htmlreport.py --file=cppcheck.xml --report-dir=cppcheck

  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  COMMENT "Running cppcheck"
  )

message(STATUS "====================================================================")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")

if (APPLE)
  message(STATUS "Mac SDK: ${DEV_SDK}")
endif()

message(STATUS "Building test suite: ${BUILD_TESTS}")

if (CCACHE)
  message(STATUS "CCache: ${CCACHE} (using to speed up compile + link)")
else()
  message(STATUS "CCache: OFF")
endif()

if (CODE_COVERAGE)
  message(STATUS "Code coverage: ${CODE_COVERAGE} (target: codecov)")
else()
  message(STATUS "Code coverage: ${CODE_COVERAGE}")
endif()

message(STATUS "Address sanitizer: ${ADDRESS_SANITIZER}")

if (STATIC_ANALYZER)
  message(STATUS "Static analyzer: ${STATIC_ANALYZER} (target: sa)")
else()
  message(STATUS "Static analyzer: ${STATIC_ANALYZER}")
endif()

message(STATUS "Found Qt: ${FOUND_QT} (min. v. ${QT_MINIMUM_VERSION})")
if (FOUND_QT)
  message(STATUS "  Qt5Core_DIR           ${Qt5Core_DIR}")
  message(STATUS "  Qt5Gui_DIR            ${Qt5Gui_DIR}")
  message(STATUS "  Qt5Widgets_DIR        ${Qt5Widgets_DIR}")
endif()

message(STATUS "====================================================================")
