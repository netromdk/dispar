#include "BinaryObject.h"

BinaryObject::BinaryObject(CpuType cpuType, CpuType cpuSubType, bool littleEndian, int systemBits,
                           FileType fileType)
  : cpuType_{cpuType}, cpuSubType_{cpuSubType}, littleEndian{littleEndian}, systemBits_{systemBits},
    fileType_{fileType}
{
  if (cpuType_ == CpuType::X86_64) {
    systemBits_ = 64;
  }
}

CpuType BinaryObject::cpuType() const
{
  return cpuType_;
}

void BinaryObject::setCpuType(CpuType type)
{
  cpuType_ = type;
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

QList<std::shared_ptr<Section>> BinaryObject::sections() const
{
  return sections_;
}

QList<std::shared_ptr<Section>> BinaryObject::sectionsByType(Section::Type type) const
{
  QList<std::shared_ptr<Section>> res;
  for (auto &sec : sections_) {
    if (sec->type() == type) {
      res << sec;
    }
  }
  return res;
}

std::shared_ptr<Section> BinaryObject::section(Section::Type type) const
{
  for (auto &sec : sections_) {
    if (sec->type() == type) {
      return sec;
    }
  }
  return nullptr;
}

void BinaryObject::addSection(std::shared_ptr<Section> ptr)
{
  sections_ << ptr;
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
