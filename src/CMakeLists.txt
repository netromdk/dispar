set(name bmod)

add_executable(
  ${name}

  main.cc

  Reader.h
  Reader.cc

  Section.h
  Section.cc

  BinaryObject.h
  BinaryObject.cc
  SymbolTable.h
  SymbolTable.cc
  )

qt5_use_modules(
  ${name}
  Core
  )
