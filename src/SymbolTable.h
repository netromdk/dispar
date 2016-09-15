#ifndef BMOD_SYMBOL_TABLE_H
#define BMOD_SYMBOL_TABLE_H

#include <QList>
#include <QString>

class SymbolEntry {
public:
  SymbolEntry(quint32 index, quint64 value, const QString &strValue = QString())
    : index_{index}, value_{value}, strValue{strValue}
  {
  }

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

class SymbolTable {
public:
  void addSymbol(const SymbolEntry &entry);
  QList<SymbolEntry> &symbols();
  const QList<SymbolEntry> &symbols() const;

  bool string(quint64 value, QString &str) const;

private:
  QList<SymbolEntry> entries;
};

#endif // BMOD_SYMBOL_TABLE_H
