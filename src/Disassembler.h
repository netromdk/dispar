#ifndef DISPAR_DISASSEMBLER_H
#define DISPAR_DISASSEMBLER_H

#include <QtGlobal>

#include <memory>

#include <capstone/capstone.h>

class QByteArray;

namespace dispar {

class BinaryObject;

class Disassembler {
public:
  enum class Syntax { ATT, INTEL, INTEL_MASM };

  class Result {
  public:
    Result(cs_insn *insn, size_t count);
    virtual ~Result();

    Result(const Result &other) = delete;
    Result &operator=(const Result &rhs) = delete;

    Result(Result &&other) = delete;
    Result &operator=(Result &&rhs) = delete;

    [[nodiscard]] size_t count() const;
    [[nodiscard]] cs_insn *instructions(size_t pos) const;

    /// Lines of addresses, mnemonics, and instruction strings.
    [[nodiscard]] QString toString() const;

  private:
    cs_insn *insn;
    size_t count_;
  };

  Disassembler(const BinaryObject &object, Syntax syntax = Syntax::INTEL);
  virtual ~Disassembler();

  Disassembler(const Disassembler &other) = delete;
  Disassembler &operator=(const Disassembler &rhs) = delete;

  Disassembler(Disassembler &&other) = delete;
  Disassembler &operator=(Disassembler &&rhs) = delete;

  [[nodiscard]] std::unique_ptr<Result> disassemble(const QByteArray &data,
                                                    quint64 baseAddr = 0) const;
  [[nodiscard]] std::unique_ptr<Result> disassemble(const QString &text,
                                                    quint64 baseAddr = 0) const;

  [[nodiscard]] bool valid() const;

private:
  csh handle{};
  bool valid_ = false;
};

} // namespace dispar

#endif // DISPAR_DISASSEMBLER_H
