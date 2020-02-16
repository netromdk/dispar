#include "Section.h"

#include <QObject>

namespace dispar {

Section::Section(Section::Type type, const QString &name, quint64 addr, quint64 size,
                 quint32 offset)
  : type_{type}, name_{name}, addr{addr}, size_{size}, offset_{offset}, disasm_(nullptr)
{
}

QString Section::typeName(Type type)
{
  switch (type) {
  case Section::Type::TEXT:
    return QObject::tr("Text");

  case Section::Type::SYMBOL_STUBS:
    return QObject::tr("Symbol Stubs");

  case Section::Type::SYMBOLS:
    return QObject::tr("Symbols");

  case Section::Type::DYN_SYMBOLS:
    return QObject::tr("Dynamic Symbols");

  case Section::Type::CSTRING:
    return QObject::tr("CString");

  case Section::Type::STRING:
    return QObject::tr("String");

  case Section::Type::FUNC_STARTS:
    return QObject::tr("Function Starts");

  case Section::Type::CODE_SIG:
    return QObject::tr("Code Signatures");

  case Section::Type::LC_VERSION_MIN_MACOSX:
    return QObject::tr("LC_VERSION_MIN_MACOSX");
  }

  return {};
}

Section::Type Section::type() const
{
  return type_;
}

QString Section::name() const
{
  return name_;
}

quint64 Section::address() const
{
  return addr;
}

quint64 Section::size() const
{
  return size_;
}

quint32 Section::offset() const
{
  return offset_;
}

QString Section::toString() const
{
  return QString("%1 (%2)").arg(name()).arg(typeName(type()));
}

bool Section::hasAddress(quint64 address) const
{
  return address >= this->address() && address < this->address() + size();
}

const QByteArray &Section::data() const
{
  return data_;
}

void Section::setData(const QByteArray &data)
{
  data_ = data;
}

void Section::setSubData(const QByteArray &subData, int pos)
{
  if (pos < 0 || pos > data_.size() - 1) {
    return;
  }

  data_.replace(pos, subData.size(), subData);
  modified = QDateTime::currentDateTime();

  QPair<int, int> region(pos, subData.size());
  if (!modifiedRegions_.contains(region)) {
    modifiedRegions_ << region;
  }
}

bool Section::isModified() const
{
  return !modifiedRegions_.isEmpty();
}

QDateTime Section::modifiedWhen() const
{
  return modified;
}

const QList<QPair<int, int>> &Section::modifiedRegions() const
{
  return modifiedRegions_;
}

void Section::setDisassembly(std::unique_ptr<Disassembler::Result> disasm)
{
  disasm_ = std::move(disasm);
}

Disassembler::Result *Section::disassembly() const
{
  return disasm_.get();
}

} // namespace dispar
