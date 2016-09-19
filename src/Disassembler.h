#ifndef DISPAR_DISASSEMBLER_H
#define DISPAR_DISASSEMBLER_H

#include "BinaryObject.h"

#include <capstone/capstone.h>

class Disassembler {
public:
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

  Disassembler(std::shared_ptr<BinaryObject> object);
  ~Disassembler();

  std::shared_ptr<Result> disassemble(const QByteArray &data, quint64 baseAddr = 0);

  bool valid() const;

private:
  csh handle;
  bool valid_;
};

#endif // DISPAR_DISASSEMBLER_H
