#ifndef DISPAR_SECTION_H
#define DISPAR_SECTION_H

#include <QByteArray>
#include <QDateTime>
#include <QList>
#include <QPair>
#include <QString>

#include <memory>

#include "Disassembler.h"

namespace dispar {

class Section {
public:
  enum class Type : int {
    TEXT,                  ///< Executable code (__text, .text).
    SYMBOL_STUBS,          ///< Indirect symbol stubs.
    SYMBOLS,               ///< Symbol table.
    DYN_SYMBOLS,           ///< Dynamic symbol table.
    CSTRING,               ///< Constant C strings (__cstring).
    STRING,                ///< String table constants.
    FUNC_STARTS,           ///< Function starts.
    CODE_SIG,              ///< Code signature.
    LC_VERSION_MIN_MACOSX, ///< macOS SDK min version (load command).
  };

  Section(Type type, const QString &name, quint64 addr, quint64 size, quint32 offset = 0);

  /// Get string representation of type.
  static QString typeName(Type type);

  Type type() const;
  QString name() const;
  quint64 address() const;
  quint64 size() const;
  quint32 offset() const;

  QString toString() const;

  bool hasAddress(quint64 address) const;

  const QByteArray &data() const;
  void setData(const QByteArray &data);

  void setSubData(const QByteArray &subData, int pos);
  bool isModified() const;
  QDateTime modifiedWhen() const;
  const QList<QPair<int, int>> &modifiedRegions() const;

  /// Takes ownership of \p disasm.
  void setDisassembly(std::unique_ptr<Disassembler::Result> disasm);
  Disassembler::Result *disassembly() const;

private:
  Type type_;
  QString name_;
  quint64 addr, size_;
  quint32 offset_;
  QByteArray data_;
  QList<QPair<int, int>> modifiedRegions_;
  QDateTime modified;
  std::unique_ptr<Disassembler::Result> disasm_;
};

} // namespace dispar

#endif // DISPAR_SECTION_H
