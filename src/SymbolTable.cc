#include "SymbolTable.h"

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

bool SymbolTable::operator==(const SymbolTable &other) const
{
  return symbols() == other.symbols();
}

bool SymbolTable::operator!=(const SymbolTable &other) const
{
  return !(*this == other);
}
