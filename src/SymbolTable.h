#ifndef DISPAR_SYMBOL_TABLE_H
#define DISPAR_SYMBOL_TABLE_H

#include "SymbolEntry.h"

#include <vector>

#include <QString>

class SymbolTable {
public:
  using EntryList = std::vector<SymbolEntry>;

  SymbolTable();
  SymbolTable(const SymbolTable &other);
  SymbolTable(SymbolTable &&other);

  void addSymbol(const SymbolEntry &entry);
  void addSymbol(SymbolEntry &&entry);

  EntryList &symbols();
  const EntryList &symbols() const;

  bool string(quint64 value, QString &str) const;

  SymbolTable &operator=(const SymbolTable &other);
  SymbolTable &operator=(SymbolTable &&other);
  bool operator==(const SymbolTable &other) const;
  bool operator!=(const SymbolTable &other) const;

private:
  EntryList entries;
};

#endif // DISPAR_SYMBOL_TABLE_H
