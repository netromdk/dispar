cmake_minimum_required(VERSION 3.12)
project(dispar)

set(CMAKE_COLOR_MAKEFILE ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_INCLUDE_CURRENT_DIR TRUE)
set(CMAKE_ALLOW_LOOSE_LOOP_CONSTRUCTS TRUE)

set(
  CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH}
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake
  ${CMAKE_CURRENT_SOURCE_DIR}/misc  # To find "MacOSXBundleInfo.plist.in"
  )

include(policies NO_POLICY_SCOPE)

option(BUILD_TESTS "Build test suite" OFF)
option(USE_CCACHE "Use ccache (if found) to speed up compile and link" OFF)
option(CODE_COVERAGE "Instrument for code coverage (clang only and builds tests!)" OFF)
option(ADDRESS_SANITIZER "Use memory error detector  (clang only and implies debug mode!)" OFF)
option(STATIC_ANALYZER "Do static analysis (clang only and implies debug mode!)" OFF)
option(EVERY_WARNING "Show every(!) warning (clang only!)" OFF)
option(TIME_TRACE "Trace compilation time into .json files (clang only!)" OFF)
option(TIME_REPORT "Trace compilation time report (clang and gcc only!)" OFF)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Output compile_commands.json (Ninja generator only!)
set(CMAKE_EXPORT_COMPILE_COMMANDS YES)

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}")

include(misc)
include(platform)
include(target)

include(ProcessorCount)
ProcessorCount(CPU_COUNT)
if (${CPU_COUNT} EQUAL 0)
  message("Could not determine processor count! Defaulting to 1.")
  set(CPU_COUNT 1)
endif()

if (CODE_COVERAGE)
  if (NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    message(FATAL_ERROR "Clang is required to do code coverage!")
  endif()

  if (NOT BUILD_TESTS)
    message("Enabling building of tests since they are required for code coverage!")
    set(BUILD_TESTS ON)
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

if (EVERY_WARNING)
  if (NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    message(FATAL_ERROR "Clang is required to use -Weverything!")
  endif()

  # But ignore some warnings.
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weverything -Wno-c++98-compat -Wno-padded -Wno-shadow-field-in-constructor -Wno-c++98-compat-pedantic -Wno-sign-conversion -Wno-old-style-cast -Wno-switch-enum -Wno-float-conversion -Wno-documentation-unknown-command -Wno-global-constructors -Wno-exit-time-destructors -Wno-return-std-move-in-c++11 -Wno-missing-prototypes -Wno-date-time")
endif()

if (TIME_TRACE)
  if (NOT "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    message(FATAL_ERROR "Clang is required to use -ftime-trace!")
  endif()

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftime-trace")
endif()

if (TIME_REPORT)
  if (NOT ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU"))
    message(FATAL_ERROR "Clang or GCC is required to use -ftime-report!")
  endif()

  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ftime-report")
endif()

# Detect major and minor program version from "Version.h".
file(
  STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/src/Version.h" _versions
  REGEX "_VERSION ="
  )
list(GET _versions 0 _version)
string(REGEX MATCH "[0-9]+" num ${_version})
set(MAJOR_VERSION ${num})
list(GET _versions 1 _version)
string(REGEX MATCH "[0-9]+" num ${_version})
set(MINOR_VERSION ${num})

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

if (APPLE)
  set(INSTALL_DIR "dispar.app")
else()
  set(INSTALL_DIR "dispar")
endif()
set(DIST_ZIP "dispar-${MAJOR_VERSION}.${MINOR_VERSION}-${CMAKE_HOST_SYSTEM_NAME}.zip")
string(TOLOWER ${DIST_ZIP} DIST_ZIP)
add_custom_target(
  dist
  ${CMAKE_COMMAND} -E tar c ${DIST_ZIP} --format=zip -- ${INSTALL_DIR}
  USES_TERMINAL
  DEPENDS install
  COMMENT "Generating distribution: ${DIST_ZIP}"
  )

# Requires llvm/clang v4+!
# Setup: CC=clang-10 CXX=clang++ cmake -G Ninja -DCODE_COVERAGE=ON ../../
# Setup: CC=clang-mp-5.0 CXX=clang++-mp-5.0 cmake -G Ninja -DCODE_COVERAGE=ON -DLLVM_PROFDATA=llvm-profdata-mp-5.0 -DLLVM_COV=llvm-cov-mp-5.0 ../../
if (CODE_COVERAGE)
  add_custom_target(
    codecov

    sh ${CMAKE_SOURCE_DIR}/scripts/code_coverage.sh "${CMAKE_CURRENT_BINARY_DIR}" "$<TARGET_FILE:${testName}>" "${LLVM_PROFDATA}" "${LLVM_COV}"

    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    USES_TERMINAL
    COMMENT "Generating test code coverage report"
    DEPENDS ${testName}
    )
endif()

# Setup: scan-build cmake -G Ninja -DSTATIC_ANALYZER=ON ../../
# Setup: scan-build-mp-5.0 cmake -DSTATIC_ANALYZER=ON -DSCAN_BUILD=scan-build-mp-5.0 ../../
if (STATIC_ANALYZER)
  add_custom_target(
    sa

    sh ${CMAKE_SOURCE_DIR}/scripts/static_analysis.sh ${SCAN_BUILD} ${CMAKE_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR} ${Qt5Core_DIR}/../../ ${binName}

    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    USES_TERMINAL
    COMMENT "Doing static analysis"
    )
endif()

# cppcheck can be cloned and compiled: git clone git@github.com:danmar/cppcheck.git
add_custom_target(
  cppcheck

  cppcheck -j ${CPU_COUNT} --enable=all --std=c++17 --suppress=*:${CMAKE_SOURCE_DIR}/lib/* --suppress=*:${CMAKE_SOURCE_DIR}/tests/gtest/* --project=${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json --xml 2> cppcheck.xml && python ${CMAKE_SOURCE_DIR}/scripts/cppcheck-htmlreport.py --file=cppcheck.xml --report-dir=cppcheck

  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  USES_TERMINAL
  COMMENT "Running cppcheck"
  )

message(STATUS "====================================================================")
message(STATUS "Program version: ${MAJOR_VERSION}.${MINOR_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")

if (APPLE)
  message(STATUS "Mac SDK: ${DEV_SDK}")
endif()

message(STATUS "Detected processor count: ${CPU_COUNT}")
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

if (EVERY_WARNING)
  message(STATUS "Every warning: ${EVERY_WARNING} (-Weverything)")
else()
  message(STATUS "Every warning: OFF")
endif()

if (TIME_TRACE)
  message(STATUS "Compilation time trace: ${TIME_TRACE} (-ftime-trace)")
else()
  message(STATUS "Compilation time trace: OFF")
endif()

if (TIME_REPORT)
  message(STATUS "Compilation time report: ${TIME_REPORT} (-ftime-report)")
else()
  message(STATUS "Compilation time report: OFF")
endif()

message(STATUS "Found Qt: ${FOUND_QT} (min. v. ${QT_MINIMUM_VERSION})")
if (FOUND_QT)
  message(STATUS "  Qt5Core_DIR           ${Qt5Core_DIR}")
  message(STATUS "  Qt5Gui_DIR            ${Qt5Gui_DIR}")
  message(STATUS "  Qt5Widgets_DIR        ${Qt5Widgets_DIR}")
  if (APPLE)
    message(STATUS "  Qt5PrintSupport_DIR   ${Qt5PrintSupport_DIR}")
  endif()
  if (APPLE OR LINUX)
    message(STATUS "  Qt5DBus_DIR           ${Qt5DBus_DIR}")
  endif()
endif()

message(STATUS "====================================================================")
