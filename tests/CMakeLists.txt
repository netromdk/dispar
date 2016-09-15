include_directories(
  gtest/
  gtest/include
  )

add_library(
  gtest
  gtest/src/gtest-all.cc
  )

set(testName tests)

add_executable(
  ${testName}

  Reader.cc

  ../src/Reader.cc
  )

add_test(
  NAME ${testName}
  COMMAND ${testName}
  )

target_link_libraries(
  ${testName}
  gtest
  )

include_directories(
  gtest/include
  ../src
  )

qt5_use_modules(
  ${testName}
  Core
  )
