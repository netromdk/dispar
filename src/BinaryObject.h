#ifndef BMOD_BINARY_OBJECT_H
#define BMOD_BINARY_OBJECT_H

#include <QList>

#include <memory>

#include "Section.h"
#include "CpuType.h"
#include "FileType.h"
#include "SymbolTable.h"

class BinaryObject {
public:
  BinaryObject(CpuType cpuType = CpuType::X86, CpuType cpuSubType = CpuType::I386,
               bool littleEndian = true, int systemBits = 32,
               FileType fileType = FileType::Execute);

  CpuType cpuType() const;
  void setCpuType(CpuType type);

  CpuType cpuSubType() const;
  void setCpuSubType(CpuType type);

  bool isLittleEndian() const;
  void setLittleEndian(bool little);

  int systemBits() const;
  void setSystemBits(int bits);

  FileType fileType() const;
  void setFileType(FileType type);

  QList<std::shared_ptr<Section>> sections() const;
  QList<std::shared_ptr<Section>> sectionsByType(Section::Type type) const;
  std::shared_ptr<Section> section(Section::Type type) const;
  void addSection(std::shared_ptr<Section> ptr);

  void setSymbolTable(const SymbolTable &tbl);
  const SymbolTable &symbolTable() const;

  void setDynSymbolTable(const SymbolTable &tbl);
  const SymbolTable &dynSymbolTable() const;

private:
  CpuType cpuType_, cpuSubType_;
  bool littleEndian;
  int systemBits_;
  FileType fileType_;
  QList<std::shared_ptr<Section>> sections_;
  SymbolTable symTable, dynsymTable;
};

#endif // BMOD_BINARY_OBJECT_H
