#include "BinaryObject.h"

BinaryObject::BinaryObject(CpuType cpuType, CpuType cpuSubType,
                           bool littleEndian, int systemBits, FileType fileType)
  : cpuType{cpuType}, cpuSubType{cpuSubType}, littleEndian{littleEndian},
  systemBits{systemBits}, fileType{fileType}
{
  if (cpuType == CpuType::X86_64) {
    this->systemBits = 64;
  }
}

CpuType BinaryObject::getCpuType() const {
  return cpuType;
}

void BinaryObject::setCpuType(CpuType type) {
  cpuType = type;
}

CpuType BinaryObject::getCpuSubType() const {
  return cpuSubType;
}

void BinaryObject::setCpuSubType(CpuType type) {
  cpuSubType = type;
}

bool BinaryObject::isLittleEndian() const {
  return littleEndian;
}

void BinaryObject::setLittleEndian(bool little) {
  littleEndian = little;
}

int BinaryObject::getSystemBits() const {
  return systemBits;
}

void BinaryObject::setSystemBits(int bits) {
  systemBits = bits;
}

FileType BinaryObject::getFileType() const {
  return fileType;
}

void BinaryObject::setFileType(FileType type) {
  fileType = type;
}

QList<std::shared_ptr<Section>> BinaryObject::getSections() const {
  return sections;
}

QList<std::shared_ptr<Section>> BinaryObject::getSectionsByType(Section::Type type) const {
  QList<std::shared_ptr<Section>> res;
  foreach (auto sec, sections) {
    if (sec->getType() == type) {
      res << sec;
    }
  }
  return res;
}

std::shared_ptr<Section> BinaryObject::getSection(Section::Type type) const {
  foreach (auto sec, sections) {
    if (sec->getType() == type) {
      return sec;
    }
  }
  return nullptr;
}

void BinaryObject::addSection(std::shared_ptr<Section> ptr) {
  sections << ptr;
}

void BinaryObject::setSymbolTable(const SymbolTable &tbl) {
  symTable = tbl;
}

const SymbolTable &BinaryObject::getSymbolTable() const {
  return symTable;
}

void BinaryObject::setDynSymbolTable(const SymbolTable &tbl) {
  dynsymTable = tbl;
}

const SymbolTable &BinaryObject::getDynSymbolTable() const {
  return dynsymTable;
}
