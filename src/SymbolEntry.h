#ifndef DISPAR_SYMBOL_ENTRY_H
#define DISPAR_SYMBOL_ENTRY_H

#include <QString>

namespace dispar {

class SymbolEntry {
public:
  SymbolEntry() = default;
  SymbolEntry(quint32 index, quint64 value, const QString &strValue = {});
  SymbolEntry(const SymbolEntry &other);
  SymbolEntry(SymbolEntry &&other) noexcept;

  [[nodiscard]] quint32 index() const;

  void setValue(quint64 value);
  [[nodiscard]] quint64 value() const;

  void setString(const QString &str);
  [[nodiscard]] const QString &string() const;

  SymbolEntry &operator=(const SymbolEntry &other);
  SymbolEntry &operator=(SymbolEntry &&other) noexcept;
  bool operator==(const SymbolEntry &other) const;
  bool operator!=(const SymbolEntry &other) const;

private:
  quint32 index_ = 0; // of string table
  quint64 value_ = 0; // of symbol
  QString strValue;   // String table value
};

} // namespace dispar

uint qHash(const dispar::SymbolEntry &entry, uint seed = 0);

#endif // DISPAR_SYMBOL_ENTRY_H
