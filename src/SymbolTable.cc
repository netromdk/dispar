#include "SymbolTable.h"

quint32 SymbolEntry::getIndex() const {
  return index;
}

void SymbolEntry::setValue(quint64 value) {
  this->value = value;
}

quint64 SymbolEntry::getValue() const {
  return value;
}

void SymbolEntry::setString(const QString &str) {
  strValue = str;
}

const QString &SymbolEntry::getString() const {
  return strValue;
}

void SymbolTable::addSymbol(const SymbolEntry &entry) {
  entries << entry;
}

bool SymbolTable::getString(quint64 value, QString &str) const {
  foreach (const auto &entry, entries) {
    if (entry.getValue() == value) {
      const auto &s  = entry.getString();
      if (s.isEmpty()) continue;
      str = s;
      return true;
    }
  }
  return false;
}

QList<SymbolEntry> &SymbolTable::getSymbols() {
  return entries;
}

const QList<SymbolEntry> &SymbolTable::getSymbols() const {
  return entries;
}
