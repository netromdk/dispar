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

  CpuType getCpuType() const;
  void setCpuType(CpuType type);

  CpuType getCpuSubType() const;
  void setCpuSubType(CpuType type);

  bool isLittleEndian() const;
  void setLittleEndian(bool little);

  int getSystemBits() const;
  void setSystemBits(int bits);

  FileType getFileType() const;
  void setFileType(FileType type);

  QList<std::shared_ptr<Section>> getSections() const;
  QList<std::shared_ptr<Section>> getSectionsByType(Section::Type type) const;
  std::shared_ptr<Section> getSection(Section::Type type) const;
  void addSection(std::shared_ptr<Section> ptr);

  void setSymbolTable(const SymbolTable &tbl);
  const SymbolTable &getSymbolTable() const;

  void setDynSymbolTable(const SymbolTable &tbl);
  const SymbolTable &getDynSymbolTable() const;

private:
  CpuType cpuType, cpuSubType;
  bool littleEndian;
  int systemBits;
  FileType fileType;
  QList<std::shared_ptr<Section>> sections;
  SymbolTable symTable, dynsymTable;
};

#endif // BMOD_BINARY_OBJECT_H
