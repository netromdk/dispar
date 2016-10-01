include_directories(
  ${CMAKE_SOURCE_DIR}/lib
  ${CMAKE_SOURCE_DIR}/lib/capstone/include
  )

LIBIBERTY_ADD_DEFINITIONS()

add_library(
  ${libName}

  Util.h
  Util.cc

  Context.h
  Context.cc
  Project.h
  Project.cc

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
  widgets/ConversionHelper.h
  widgets/ConversionHelper.cc
  widgets/DisassemblerDialog.h
  widgets/DisassemblerDialog.cc
  widgets/OptionsDialog.h
  widgets/OptionsDialog.cc

  formats/Format.h
  formats/Format.cc
  formats/FormatLoader.h
  formats/FormatLoader.cc
  formats/MachO.h
  formats/MachO.cc
  )

target_link_libraries(
  ${libName}
  capstone-static
  iberty
  )

qt5_use_modules(
  ${libName}
  Core
  Gui
  Widgets
  )

add_dispar_executable(
  ${binName}

  main.cc
  )

target_link_libraries(
  ${binName}
  ${libName}
  )
