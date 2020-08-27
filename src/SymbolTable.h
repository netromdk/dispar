#ifndef DISPAR_SYMBOL_TABLE_H
#define DISPAR_SYMBOL_TABLE_H

#include "SymbolEntry.h"

#include <vector>

#include <QString>

namespace dispar {

class SymbolTable {
public:
  using EntryList = std::vector<SymbolEntry>;

  SymbolTable() = default;
  SymbolTable(const SymbolTable &other) = default;
  SymbolTable(SymbolTable &&other) = default;

  void addSymbol(const SymbolEntry &entry);
  void addSymbol(SymbolEntry &&entry);

  EntryList &symbols();
  [[nodiscard]] const EntryList &symbols() const;

  bool string(quint64 value, QString &str) const;

  SymbolTable &operator=(const SymbolTable &other);
  SymbolTable &operator=(SymbolTable &&other) noexcept;
  bool operator==(const SymbolTable &other) const;
  bool operator!=(const SymbolTable &other) const;

private:
  EntryList entries;
};

} // namespace dispar

#endif // DISPAR_SYMBOL_TABLE_H
