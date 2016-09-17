#ifndef DISPAR_SYMBOL_TABLE_H
#define DISPAR_SYMBOL_TABLE_H

#include "SymbolEntry.h"

#include <QList>
#include <QString>

class SymbolTable {
public:
  void addSymbol(const SymbolEntry &entry);
  QList<SymbolEntry> &symbols();
  const QList<SymbolEntry> &symbols() const;

  bool string(quint64 value, QString &str) const;

  bool operator==(const SymbolTable &other) const;
  bool operator!=(const SymbolTable &other) const;

private:
  QList<SymbolEntry> entries;
};

#endif // DISPAR_SYMBOL_TABLE_H
