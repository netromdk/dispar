#ifndef DISPAR_DISASSEMBLER_H
#define DISPAR_DISASSEMBLER_H

#include <QtGlobal>

#include <memory>

#include <capstone.h>

class QByteArray;

namespace dispar {

class BinaryObject;

class Disassembler {
public:
  enum class Syntax { ATT, INTEL };

  class Result {
  public:
    Result(cs_insn *insn, size_t count);
    ~Result();

    size_t count() const;
    cs_insn *instructions(size_t pos) const;

  private:
    cs_insn *insn;
    size_t count_;
  };

  Disassembler(const BinaryObject &object, Syntax syntax = Syntax::INTEL);
  ~Disassembler();

  std::unique_ptr<Result> disassemble(const QByteArray &data, quint64 baseAddr = 0);
  std::unique_ptr<Result> disassemble(const QString &text, quint64 baseAddr = 0);

  bool valid() const;

private:
  csh handle;
  bool valid_;
};

} // namespace dispar

#endif // DISPAR_DISASSEMBLER_H
