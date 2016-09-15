add_library(
  ${libName}

  Util.h
  Util.cc

  Reader.h
  Reader.cc

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

  formats/Format.h
  formats/Format.cc
  formats/MachO.h
  formats/MachO.cc
  )

qt5_use_modules(
  ${libName}
  Core
  )

add_executable(
  ${binName}

  main.cc
  )

target_link_libraries(
  ${binName}
  ${libName}
  )
