include_directories(
  gtest/
  gtest/include
  )

add_library(
  gtest
  gtest/src/gtest-all.cc
  )

add_executable(
  ${testName}

  main.cc

  testutils.h
  testutils.cc

  Reader.cc
  SymbolEntry.cc

  formats/Format.cc
  formats/MachO.cc

  binaries.qrc
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
