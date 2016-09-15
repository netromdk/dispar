#include "SymbolTable.h"

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

void SymbolTable::addSymbol(const SymbolEntry &entry)
{
  entries << entry;
}

bool SymbolTable::string(quint64 value, QString &str) const
{
  for (const auto &entry : entries) {
    if (entry.value() == value) {
      const auto &s = entry.string();
      if (s.isEmpty()) continue;
      str = s;
      return true;
    }
  }
  return false;
}

QList<SymbolEntry> &SymbolTable::symbols()
{
  return entries;
}

const QList<SymbolEntry> &SymbolTable::symbols() const
{
  return entries;
}
