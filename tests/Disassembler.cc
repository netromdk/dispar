#include "gtest/gtest.h"

#include "Disassembler.h"

TEST(Disassembler, instantiate)
{
  auto obj = std::make_shared<BinaryObject>();
  Disassembler dis(obj);
}

TEST(Disassembler, valid)
{
  {
    Disassembler dis(nullptr);
    EXPECT_FALSE(dis.valid());
  }

  {
    auto obj = std::make_shared<BinaryObject>();
    Disassembler dis(obj);
    EXPECT_TRUE(dis.valid());
  }

  {
    auto obj = std::make_shared<BinaryObject>(CpuType::X86_64, CpuType::I386, true, 64);
    Disassembler dis(obj);
    EXPECT_TRUE(dis.valid());
  }
}

TEST(Disassembler, disassemble)
{
  {
    auto obj = std::make_shared<BinaryObject>();
    Disassembler dis(obj);
    ASSERT_TRUE(dis.valid());

    const char *code = "\x90";
    auto res = dis.disassemble(code, 1);
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->count(), 1);

    auto *instr = res->instructions(0);
    ASSERT_NE(instr, nullptr);
    EXPECT_EQ(std::string(instr->mnemonic), std::string("nop")) << instr->mnemonic;
  }

  {
    auto obj = std::make_shared<BinaryObject>();
    Disassembler dis(obj);
    ASSERT_TRUE(dis.valid());

    auto res = dis.disassemble(nullptr, 0);
    ASSERT_EQ(res, nullptr);
  }
}
