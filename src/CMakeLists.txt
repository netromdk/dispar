include_directories(
  ${CAPSTONE_INC_DIR}
  ${CMAKE_SOURCE_DIR}/lib
  )

if (NOT WIN32)
  include_directories(
    /usr/local/include
    )
endif()

LIBIBERTY_ADD_DEFINITIONS()

add_library(
  ${libName}

  Util.h
  Util.cc

  Reader.h
  Reader.cc
  CStringReader.h
  CStringReader.cc

  Disassembler.h
  Disassembler.cc

  Section.h
  Section.cc

  CpuType.h
  CpuType.cc
  FileType.h
  FileType.cc

  BinaryObject.h
  BinaryObject.cc
  SymbolEntry.h
  SymbolEntry.cc
  SymbolTable.h
  SymbolTable.cc

  widgets/MainWindow.h
  widgets/MainWindow.cc
  widgets/BinaryWidget.h
  widgets/BinaryWidget.cc
  widgets/PersistentSplitter.h
  widgets/PersistentSplitter.cc
  widgets/AboutDialog.h
  widgets/AboutDialog.cc

  formats/Format.h
  formats/Format.cc
  formats/MachO.h
  formats/MachO.cc
  )

target_link_libraries(
  ${libName}
  ${CAPSTONE_STATIC_LIB}
  iberty
  )

qt5_use_modules(
  ${libName}
  Core
  Gui
  Widgets
  )

add_executable(
  ${binName}

  main.cc
  )

target_link_libraries(
  ${binName}
  ${libName}
  )
