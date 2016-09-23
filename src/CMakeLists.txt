include_directories(
  ${CAPSTONE_INC_DIR}
  ${LIBIBERTY_INC_DIR}
  )

if (NOT WIN32)
  include_directories(
    /usr/local/include
    )
endif()

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

  formats/Format.h
  formats/Format.cc
  formats/MachO.h
  formats/MachO.cc
  )

target_link_libraries(
  ${libName}
  ${CAPSTONE_STATIC_LIB}
  ${LIBIBERTY_STATIC_LIB}
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
