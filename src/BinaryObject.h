#ifndef DISPAR_BINARY_OBJECT_H
#define DISPAR_BINARY_OBJECT_H

#include <QList>

#include <memory>
#include <vector>

#include "CpuType.h"
#include "FileType.h"
#include "Section.h"
#include "SymbolTable.h"

class BinaryObject {
public:
  BinaryObject(CpuType cpuType = CpuType::X86, CpuType cpuSubType = CpuType::I386,
               bool littleEndian = true, int systemBits = 32,
               FileType fileType = FileType::EXECUTE);

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

  /// Takes ownership of \p section.
  void addSection(std::unique_ptr<Section> section);

  QList<Section *> sections() const;
  QList<Section *> sectionsByType(Section::Type type) const;

  /// First section of \p type.
  /** Returns \p nullptr if none were found. */
  Section *section(Section::Type type) const;

  void setSymbolTable(const SymbolTable &table);
  void setSymbolTable(SymbolTable &&table);
  const SymbolTable &symbolTable() const;

  void setDynSymbolTable(const SymbolTable &table);
  void setDynSymbolTable(SymbolTable &&table);
  const SymbolTable &dynSymbolTable() const;

private:
  CpuType cpuType_, cpuSubType_;
  bool littleEndian;
  int systemBits_;
  FileType fileType_;
  std::vector<std::unique_ptr<Section>> sections_;
  SymbolTable symTable, dynsymTable;
};

#endif // DISPAR_BINARY_OBJECT_H
