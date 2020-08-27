#include "SymbolEntry.h"

#include <QHash>

namespace dispar {

SymbolEntry::SymbolEntry() : index_(0), value_(0), strValue()
{
}

SymbolEntry::SymbolEntry(quint32 index, quint64 value, const QString &strValue_)
  : index_{index}, value_{value}, strValue{strValue_}
{
}

SymbolEntry::SymbolEntry(SymbolEntry &&other) : SymbolEntry()
{
  *this = std::move(other);
}

SymbolEntry::SymbolEntry(const SymbolEntry &other)
{
  *this = other;
}

quint32 SymbolEntry::index() const
{
  return index_;
}

void SymbolEntry::setValue(quint64 value)
{
  value_ = value;
}

quint64 SymbolEntry::value() const
{
  return value_;
}

void SymbolEntry::setString(const QString &str)
{
  strValue = str;
}

const QString &SymbolEntry::string() const
{
  return strValue;
}

SymbolEntry &SymbolEntry::operator=(const SymbolEntry &other) = default;

SymbolEntry &SymbolEntry::operator=(SymbolEntry &&other)
{
  std::swap(index_, other.index_);
  std::swap(value_, other.value_);
  strValue = std::move(other.strValue);
  return *this;
}

bool SymbolEntry::operator==(const SymbolEntry &other) const
{
  return index() == other.index() && value() == other.value() && string() == other.string();
}

bool SymbolEntry::operator!=(const SymbolEntry &other) const
{
  return !(*this == other);
}

} // namespace dispar

uint qHash(const dispar::SymbolEntry &entry, uint seed)
{
  return qHash(QString("%1,%2,%3").arg(entry.index()).arg(entry.value()).arg(entry.string()), seed);
}
