#ifndef DISPAR_SECTION_H
#define DISPAR_SECTION_H

#include <QByteArray>
#include <QDateTime>
#include <QList>
#include <QString>

#include <memory>

#include "Disassembler.h"

namespace dispar {

class Section {
public:
  enum class Type : int {
    TEXT,                    ///< Executable code (__text, .text).
    SYMBOL_STUBS,            ///< Indirect symbol stubs.
    SYMBOLS,                 ///< Symbol table.
    DYN_SYMBOLS,             ///< Dynamic symbol table.
    CSTRING,                 ///< Constant C strings (__cstring).
    STRING,                  ///< String table constants.
    FUNC_STARTS,             ///< Function starts.
    CODE_SIG,                ///< Code signature.
    LC_VERSION_MIN_MACOSX,   ///< macOS SDK min version (load command).
    LC_VERSION_MIN_IPHONEOS, ///< iOS SDK min version (load command).
    LC_VERSION_MIN_WATCHOS,  ///< watchOS SDK min version (load command).
    LC_VERSION_MIN_TVOS,     ///< tvOS SDK min version (load command).
  };

  /// Modified region indicates where and how much data was changed but doesn't contain the data
  /// itself.
  struct ModifiedRegion {
    ModifiedRegion(int position, const QByteArray &data);

    bool operator==(const ModifiedRegion &rhs) const;
    bool operator!=(const ModifiedRegion &rhs) const;

    [[nodiscard]] int position() const;
    [[nodiscard]] int size() const;

  private:
    int position_ = -1, size_ = 0;
    QByteArray hash;
  };

  Section(Type type, const QString &name, quint64 addr, quint64 size, quint32 offset = 0);

  /// Get string representation of type.
  static QString typeName(Type type);

  [[nodiscard]] Type type() const;
  [[nodiscard]] QString name() const;
  [[nodiscard]] quint64 address() const;
  [[nodiscard]] quint64 size() const;
  [[nodiscard]] quint32 offset() const;

  [[nodiscard]] QString toString() const;

  [[nodiscard]] bool hasAddress(quint64 address) const;

  [[nodiscard]] const QByteArray &data() const;
  void setData(const QByteArray &data);

  void setSubData(const QByteArray &subData, int pos);
  [[nodiscard]] bool isModified() const;
  [[nodiscard]] QDateTime modifiedWhen() const;
  [[nodiscard]] const QList<ModifiedRegion> &modifiedRegions() const;

  /// Takes ownership of \p disasm.
  void setDisassembly(std::unique_ptr<Disassembler::Result> disasm);
  [[nodiscard]] Disassembler::Result *disassembly() const;

private:
  Type type_;
  QString name_;
  quint64 addr, size_;
  quint32 offset_;
  QByteArray data_;
  QList<ModifiedRegion> modifiedRegions_;
  QDateTime modified;
  std::unique_ptr<Disassembler::Result> disasm_;
};

} // namespace dispar

#endif // DISPAR_SECTION_H
