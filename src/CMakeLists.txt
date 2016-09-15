set(name dispar)

add_executable(
  ${name}

  main.cc

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
  SymbolTable.h
  SymbolTable.cc

  formats/Format.h
  formats/Format.cc
  formats/MachO.h
  formats/MachO.cc
  )

qt5_use_modules(
  ${name}
  Core
  )
