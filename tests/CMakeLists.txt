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
  )

add_test(
  NAME ${testName}
  COMMAND ${testName}
  )

if (NOT LINUX)
  target_link_libraries(
    ${testName}
    gtest
    ${libName}
    )
else()
  target_link_libraries(
    ${testName}
    gtest
    ${libName}
    pthread
    )
endif()

include_directories(
  gtest/include
  ../src
  )

qt5_use_modules(
  ${testName}
  Core
  )
