include_directories(
  SYSTEM
  gtest/
  gtest/include
  ${CMAKE_SOURCE_DIR}/lib
  ${CMAKE_SOURCE_DIR}/lib/capstone/include
  )

add_library(
  gtest
  gtest/src/gtest-all.cc
  )

disable_all_warnings(gtest)

add_custom_target(tests)

macro(add_dispar_test name)
  set(targetName "test_${name}")
  add_dispar_executable(
    ${targetName}

    main.cc

    testutils.h
    testutils.cc

    binaries.qrc

    ${ARGN}
    )

  add_dependencies(tests ${targetName})

  add_test(
    NAME ${name}
    COMMAND ${targetName}
    )

  target_link_libraries(
    ${targetName}
    gtest
    ${libName}
    )

  if (LINUX)
    target_link_libraries(
      ${targetName}
      pthread
      )
  endif()

  include_directories(
    gtest/include
    ../src
    )

  qt5_use_modules(
    ${targetName}
    Core
    )
endmacro()

add_dispar_test(
  general

  Version.cc
  Util.cc
  Project.cc
  )

add_dispar_test(
  binary

  Reader.cc
  CStringReader.cc

  CpuType.cc

  Disassembler.cc

  SymbolEntry.cc
  SymbolTable.cc
  Section.cc
  BinaryObject.cc
  )

add_dispar_test(
  formats

  formats/Format.cc
  formats/FormatLoader.cc
  )

add_dispar_test(
  formats_macho

  formats/MachO.cc
  )

add_dispar_test(
  widgets

  widgets/NumberValidator.cc
  widgets/AsciiValidator.cc
  )

add_dispar_test(
  misc

  Debugger.cc
  AddrHexAsciiEncoder.cc
  MacSdkVersionPatcher.cc
  )
