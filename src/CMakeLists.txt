add_library(
  ${libName}

  Util.h
  Util.cc

  Reader.h
  Reader.cc
  CStringReader.h
  CStringReader.cc

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

  formats/Format.h
  formats/Format.cc
  formats/MachO.h
  formats/MachO.cc
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
