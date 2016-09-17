#ifndef DISPAR_SECTION_H
#define DISPAR_SECTION_H

#include <QList>
#include <QPair>
#include <QString>
#include <QDateTime>
#include <QByteArray>

#include <memory>

class Section {
public:
  enum class Type : int {
    Text,        ///< Executable code (__text, .text).
    SymbolStubs, ///< Indirect symbol stubs.
    Symbols,     ///< Symbol table.
    DynSymbols,  ///< Dynamic symbol table.
    CString,     ///< Constant C strings (__cstring).
    String,      ///< String table constants.
    FuncStarts,  ///< Function starts.
    CodeSig      ///< Code signature.
  };

  Section(Type type, const QString &name, quint64 addr, quint64 size, quint32 offset = 0);

  /// Get string representation of type.
  static QString typeName(Type type);

  Type type() const;
  QString name() const;
  quint64 address() const;
  quint64 size() const;
  quint32 offset() const;

  const QByteArray &data() const;
  void setData(const QByteArray &data);

  void setSubData(const QByteArray &subData, int pos);
  bool isModified() const;
  QDateTime modifiedWhen() const;
  const QList<QPair<int, int>> &modifiedRegions() const;

private:
  Type type_;
  QString name_;
  quint64 addr, size_;
  quint32 offset_;
  QByteArray data_;
  QList<QPair<int, int>> modifiedRegions_;
  QDateTime modified;
};

#endif // DISPAR_SECTION_H
