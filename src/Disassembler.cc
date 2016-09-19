#include "Disassembler.h"

#include <QDebug>

Disassembler::Result::Result(cs_insn *insn, size_t count) : insn(insn), count_(count)
{
}

Disassembler::Result::~Result()
{
  if (insn) {
    cs_free(insn, count_);
  }
}

size_t Disassembler::Result::count() const
{
  return count_;
}

cs_insn *Disassembler::Result::instructions(size_t pos) const
{
  Q_ASSERT(pos < count());
  return insn + pos;
}

Disassembler::Disassembler(std::shared_ptr<BinaryObject> object) : valid_(false)
{
  if (!object) {
    qCritical() << "Null binary object!";
    return;
  }

  cs_arch arch;
  switch (object->cpuType()) {
  case CpuType::X86:
  case CpuType::X86_64:
    arch = cs_arch::CS_ARCH_X86;
    break;

  default:
    qCritical() << "Invalid arch:" << cpuTypeName(object->cpuType());
    return;
  }

  int mode = (object->systemBits() == 32 ? cs_mode::CS_MODE_32 : cs_mode::CS_MODE_64);
  mode += (object->isLittleEndian() ? cs_mode::CS_MODE_LITTLE_ENDIAN : cs_mode::CS_MODE_BIG_ENDIAN);

  cs_err err = cs_open(arch, static_cast<cs_mode>(mode), &handle);
  if (err) {
    qCritical() << "Failed to create cs disassembler!" << (int) err;
    return;
  }

  valid_ = !cs_option(handle, cs_opt_type::CS_OPT_DETAIL, cs_opt_value::CS_OPT_ON);
  valid_ &= !cs_option(handle, cs_opt_type::CS_OPT_SYNTAX, cs_opt_value::CS_OPT_SYNTAX_INTEL);
}

Disassembler::~Disassembler()
{
  if (valid()) {
    cs_close(&handle);
  }
}

std::shared_ptr<Disassembler::Result> Disassembler::disassemble(const QByteArray &data,
                                                                quint64 baseAddr)
{
  const void *code = data.constData();
  cs_insn *insn = nullptr;
  size_t count =
    cs_disasm(handle, static_cast<const unsigned char *>(code), data.size(), baseAddr, 0, &insn);
  if (count == 0) {
    return nullptr;
  }
  return std::make_shared<Result>(insn, count);
}

bool Disassembler::valid() const
{
  return valid_;
}
