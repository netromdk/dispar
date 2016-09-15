include_directories(
  gtest/
  gtest/include
  )

add_library(
  gtest
  gtest/src/gtest-all.cc
  )

macro(add_dispar_test name)
  add_executable(
    ${name}
    ${ARGV}
    )

  add_test(
    NAME ${name}
    COMMAND ${name}
    )

  target_link_libraries(
    ${name}
    gtest
    )

  include_directories(
    gtest/include
    ../src
    )

  qt5_use_modules(
    ${name}
    Core
    )
endmacro(add_dispar_test)

add_dispar_test(
  Reader
  Reader.cc

  ../src/Reader.cc
  )
