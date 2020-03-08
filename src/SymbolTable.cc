#include "SymbolTable.h"

namespace dispar {

void SymbolTable::addSymbol(const SymbolEntry &entry)
{
  entries.push_back(entry);
}

void SymbolTable::addSymbol(SymbolEntry &&entry)
{
  entries.emplace_back(std::move(entry));
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

SymbolTable::EntryList &SymbolTable::symbols()
{
  return entries;
}

const SymbolTable::EntryList &SymbolTable::symbols() const
{
  return entries;
}

SymbolTable &SymbolTable::operator=(const SymbolTable &other)
{
  entries = other.entries;
  return *this;
}

SymbolTable &SymbolTable::operator=(SymbolTable &&other)
{
  entries = std::move(other.entries);
  return *this;
}

bool SymbolTable::operator==(const SymbolTable &other) const
{
  return symbols() == other.symbols();
}

bool SymbolTable::operator!=(const SymbolTable &other) const
{
  return !(*this == other);
}

} // namespace dispar
