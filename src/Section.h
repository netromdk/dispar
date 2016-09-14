#ifndef BMOD_SECTION_H
#define BMOD_SECTION_H

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
    CodeSig,     ///< Code signature.
  };

  Section(Type type, const QString &name, quint64 addr, quint64 size,
          quint32 offset = 0);

  Type getType() const;
  QString getName() const;
  quint64 getAddress() const;
  quint64 getSize() const;
  quint32 getOffset() const;

  const QByteArray &getData() const;
  void setData(const QByteArray &data);

  void setSubData(const QByteArray &subData, int pos);
  bool isModified() const;
  QDateTime modifiedWhen() const;
  const QList<QPair<int, int>> &getModifiedRegions() const;

private:
  Type type;
  QString name;
  quint64 addr, size;
  quint32 offset;
  QByteArray data;
  QList<QPair<int, int>> modifiedRegions;
  QDateTime modified;
};

#endif // BMOD_SECTION_H
