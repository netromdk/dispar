#include "SymbolEntry.h"

#include <QHash>

SymbolEntry::SymbolEntry(quint32 index, quint64 value, const QString &strValue)
  : index_{index}, value_{value}, strValue{strValue}
{
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

bool SymbolEntry::operator==(const SymbolEntry &other) const
{
  return index() == other.index() && value() == other.value() && string() == other.string();
}

bool SymbolEntry::operator!=(const SymbolEntry &other) const
{
  return !(*this == other);
}

uint qHash(const SymbolEntry &entry, uint seed)
{
  return qHash(QString("%1,%2,%3").arg(entry.index()).arg(entry.value()).arg(entry.string()), seed);
}
