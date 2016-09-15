#ifndef DISPAR_SYMBOL_ENTRY_H
#define DISPAR_SYMBOL_ENTRY_H

#include <QString>

class SymbolEntry {
public:
  SymbolEntry(quint32 index, quint64 value, const QString &strValue = QString());

  quint32 index() const;

  void setValue(quint64 value);
  quint64 value() const;

  void setString(const QString &str);
  const QString &string() const;

private:
  quint32 index_;   // of string table
  quint64 value_;   // of symbol
  QString strValue; // String table value
};

#endif // DISPAR_SYMBOL_ENTRY_H
