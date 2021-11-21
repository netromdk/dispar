#ifndef DISPAR_BINARY_OBJECT_H
#define DISPAR_BINARY_OBJECT_H

#include <QList>

#include <memory>
#include <vector>

#include "Constants.h"
#include "CpuType.h"
#include "FileType.h"
#include "Section.h"
#include "SymbolTable.h"

namespace dispar {

class BinaryObject {
public:
  BinaryObject(CpuType cpuType = CpuType::X86, CpuType cpuSubType = CpuType::I386,
               Constants::Endianness endianness = Constants::Endianness::Little,
               int systemBits = 32, FileType fileType = FileType::EXECUTE);

  [[nodiscard]] CpuType cpuType() const;
  void setCpuType(CpuType type);

  [[nodiscard]] CpuType cpuSubType() const;
  void setCpuSubType(CpuType type);

  [[nodiscard]] Constants::Endianness endianness() const;

  void setEndianness(Constants::Endianness endianness);

  [[nodiscard]] int systemBits() const;
  void setSystemBits(int bits);

  [[nodiscard]] FileType fileType() const;
  void setFileType(FileType type);

  [[nodiscard]] QString toString() const;

  /// Takes ownership of \p section.
  void addSection(std::unique_ptr<Section> section);

  [[nodiscard]] QList<Section *> sections() const;
  [[nodiscard]] QList<Section *> sectionsByType(Section::Type type) const;
  [[nodiscard]] QList<Section *> sectionsByTypes(const QList<Section::Type> &types) const;

  /// First section of \p type.
  /** Returns \p nullptr if none were found. */
  [[nodiscard]] Section *section(Section::Type type) const;

  void setSymbolTable(const SymbolTable &table);
  void setSymbolTable(SymbolTable &&table);
  [[nodiscard]] const SymbolTable &symbolTable() const;

  void setDynSymbolTable(const SymbolTable &table);
  void setDynSymbolTable(SymbolTable &&table);
  [[nodiscard]] const SymbolTable &dynSymbolTable() const;

private:
  CpuType cpuType_, cpuSubType_;
  Constants::Endianness endianness_;
  int systemBits_;
  FileType fileType_;
  std::vector<std::unique_ptr<Section>> sections_;
  SymbolTable symTable, dynsymTable;
};

} // namespace dispar

#endif // DISPAR_BINARY_OBJECT_H
