#ifndef DISPAR_SYMBOL_ENTRY_H
#define DISPAR_SYMBOL_ENTRY_H

#include <QString>

class SymbolEntry {
public:
  SymbolEntry();
  SymbolEntry(quint32 index, quint64 value, const QString &strValue = {});
  SymbolEntry(const SymbolEntry &other);
  SymbolEntry(SymbolEntry &&other);

  quint32 index() const;

  void setValue(quint64 value);
  quint64 value() const;

  void setString(const QString &str);
  const QString &string() const;

  SymbolEntry &operator=(const SymbolEntry &other);
  SymbolEntry &operator=(SymbolEntry &&other);
  bool operator==(const SymbolEntry &other) const;
  bool operator!=(const SymbolEntry &other) const;

private:
  quint32 index_;   // of string table
  quint64 value_;   // of symbol
  QString strValue; // String table value
};

uint qHash(const SymbolEntry &entry, uint seed = 0);

#endif // DISPAR_SYMBOL_ENTRY_H
