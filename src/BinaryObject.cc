#include "BinaryObject.h"
#include "CpuType.h"
#include "cxx.h"

namespace dispar {

BinaryObject::BinaryObject(CpuType cpuType, CpuType cpuSubType, bool littleEndian_, int systemBits,
                           FileType fileType)
  : cpuType_{cpuType}, cpuSubType_{cpuSubType}, littleEndian{littleEndian_},
    systemBits_{systemBits}, fileType_{fileType}
{
  setCpuType(cpuType);
}

CpuType BinaryObject::cpuType() const
{
  return cpuType_;
}

void BinaryObject::setCpuType(CpuType type)
{
  cpuType_ = type;
  if (cpuType_ == CpuType::X86_64) {
    systemBits_ = 64;
  }
}

CpuType BinaryObject::cpuSubType() const
{
  return cpuSubType_;
}

void BinaryObject::setCpuSubType(CpuType type)
{
  cpuSubType_ = type;
}

bool BinaryObject::isLittleEndian() const
{
  return littleEndian;
}

void BinaryObject::setLittleEndian(bool little)
{
  littleEndian = little;
}

int BinaryObject::systemBits() const
{
  return systemBits_;
}

void BinaryObject::setSystemBits(int bits)
{
  systemBits_ = bits;
}

FileType BinaryObject::fileType() const
{
  return fileType_;
}

void BinaryObject::setFileType(FileType type)
{
  fileType_ = type;
}

QString BinaryObject::toString() const
{
  return QString("%1 (%2), %3-bit, %4, %5 Endian, %6 sections")
    .arg(cpuTypeName(cpuType()))
    .arg(cpuTypeName(cpuSubType()))
    .arg(systemBits())
    .arg(fileTypeName(fileType()))
    .arg(isLittleEndian() ? "Little" : "Big")
    .arg(sections().size());
}

QList<Section *> BinaryObject::sections() const
{
  QList<Section *> res;
  for (auto &section : sections_) {
    res << section.get();
  }
  return res;
}

QList<Section *> BinaryObject::sectionsByType(Section::Type type) const
{
  QList<Section *> res;
  for (auto &section : sections_) {
    if (section->type() == type) {
      res << section.get();
    }
  }
  return res;
}

QList<Section *> BinaryObject::sectionsByTypes(const QList<Section::Type> &types) const
{
  QList<Section *> res;
  for (auto &section : sections_) {
    if (types.contains(section->type())) {
      res << section.get();
    }
  }
  return res;
}

Section *BinaryObject::section(Section::Type type) const
{
  if (auto it = cxx::find_if(sections_, [type](auto &section) { return section->type() == type; });
      it != sections_.cend()) {
    return it->get();
  }
  return nullptr;
}

void BinaryObject::addSection(std::unique_ptr<Section> section)
{
  sections_.emplace_back(std::move(section));
}

void BinaryObject::setSymbolTable(const SymbolTable &table)
{
  symTable = table;
}

void BinaryObject::setSymbolTable(SymbolTable &&table)
{
  symTable = std::move(table);
}

const SymbolTable &BinaryObject::symbolTable() const
{
  return symTable;
}

void BinaryObject::setDynSymbolTable(const SymbolTable &table)
{
  dynsymTable = table;
}

void BinaryObject::setDynSymbolTable(SymbolTable &&table)
{
  dynsymTable = std::move(table);
}

const SymbolTable &BinaryObject::dynSymbolTable() const
{
  return dynsymTable;
}

} // namespace dispar
