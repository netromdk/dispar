#include "BinaryObject.h"

BinaryObject::BinaryObject(CpuType cpuType, CpuType cpuSubType, bool littleEndian, int systemBits,
                           FileType fileType)
  : cpuType_{cpuType}, cpuSubType_{cpuSubType}, littleEndian{littleEndian},
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

Section *BinaryObject::section(Section::Type type) const
{
  for (auto &section : sections_) {
    if (section->type() == type) {
      return section.get();
    }
  }
  return nullptr;
}

void BinaryObject::addSection(std::unique_ptr<Section> section)
{
  sections_.emplace_back(std::move(section));
}

void BinaryObject::setSymbolTable(const SymbolTable &tbl)
{
  symTable = tbl;
}

const SymbolTable &BinaryObject::symbolTable() const
{
  return symTable;
}

void BinaryObject::setDynSymbolTable(const SymbolTable &tbl)
{
  dynsymTable = tbl;
}

const SymbolTable &BinaryObject::dynSymbolTable() const
{
  return dynsymTable;
}
