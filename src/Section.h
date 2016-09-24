#ifndef DISPAR_SECTION_H
#define DISPAR_SECTION_H

#include <QByteArray>
#include <QDateTime>
#include <QList>
#include <QPair>
#include <QString>

#include <memory>

#include "Disassembler.h"

class Section {
public:
  enum class Type : int {
    TEXT,         ///< Executable code (__text, .text).
    SYMBOL_STUBS, ///< Indirect symbol stubs.
    SYMBOLS,      ///< Symbol table.
    DYN_SYMBOLS,  ///< Dynamic symbol table.
    CSTRING,      ///< Constant C strings (__cstring).
    STRING,       ///< String table constants.
    FUNC_STARTS,  ///< Function starts.
    CODE_SIG      ///< Code signature.
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

  void setDisassembly(std::shared_ptr<Disassembler::Result> disasm);
  std::shared_ptr<Disassembler::Result> disassembly() const;

private:
  Type type_;
  QString name_;
  quint64 addr, size_;
  quint32 offset_;
  QByteArray data_;
  QList<QPair<int, int>> modifiedRegions_;
  QDateTime modified;
  std::shared_ptr<Disassembler::Result> disasm;
};

#endif // DISPAR_SECTION_H
