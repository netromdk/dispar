#include "gtest/gtest.h"

#include "BinaryObject.h"
#include "CpuType.h"
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

TEST(Disassembler, disassembleData)
{
  {
    // x86 32-bit
    auto obj = std::make_shared<BinaryObject>();
    Disassembler dis(obj, Disassembler::Syntax::INTEL);
    ASSERT_TRUE(dis.valid());

    // dec eax
    // sub esp, 0x70
    const char *code = "\x48\x83\xec\x70";
    auto res = dis.disassemble(QByteArray(code));
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->count(), 2);

    auto *instr = res->instructions(0);
    ASSERT_NE(instr, nullptr);
    EXPECT_EQ(std::string(instr->mnemonic), std::string("dec")) << instr->mnemonic;
    EXPECT_EQ(std::string(instr->op_str), std::string("eax")) << instr->op_str;

    instr = res->instructions(1);
    ASSERT_NE(instr, nullptr);
    EXPECT_EQ(std::string(instr->mnemonic), std::string("sub")) << instr->mnemonic;
    EXPECT_EQ(std::string(instr->op_str), std::string("esp, 0x70")) << instr->op_str;
  }

  {
    // x86 32-bit
    auto obj = std::make_shared<BinaryObject>();
    Disassembler dis(obj, Disassembler::Syntax::ATT);
    ASSERT_TRUE(dis.valid());

    // decl %eax
    // subl $0x70, %esp
    const char *code = "\x48\x83\xec\x70";
    auto res = dis.disassemble(QByteArray(code));
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->count(), 2);

    auto *instr = res->instructions(0);
    ASSERT_NE(instr, nullptr);
    EXPECT_EQ(std::string(instr->mnemonic), std::string("decl")) << instr->mnemonic;
    EXPECT_EQ(std::string(instr->op_str), std::string("%eax")) << instr->op_str;

    instr = res->instructions(1);
    ASSERT_NE(instr, nullptr);
    EXPECT_EQ(std::string(instr->mnemonic), std::string("subl")) << instr->mnemonic;
    EXPECT_EQ(std::string(instr->op_str), std::string("$0x70, %esp")) << instr->op_str;
  }

  {
    // x86 64-bit
    auto obj = std::make_shared<BinaryObject>();
    obj->setCpuType(CpuType::X86_64);

    Disassembler dis(obj, Disassembler::Syntax::INTEL);
    ASSERT_TRUE(dis.valid());

    // sub rsp, 0x70
    const char *code = "\x48\x83\xec\x70";
    auto res = dis.disassemble(QByteArray(code));
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->count(), 1);

    auto *instr = res->instructions(0);
    ASSERT_NE(instr, nullptr);
    EXPECT_EQ(std::string(instr->mnemonic), std::string("sub")) << instr->mnemonic;
    EXPECT_EQ(std::string(instr->op_str), std::string("rsp, 0x70")) << instr->op_str;
  }

  {
    // x86 64-bit
    auto obj = std::make_shared<BinaryObject>();
    obj->setCpuType(CpuType::X86_64);

    Disassembler dis(obj, Disassembler::Syntax::ATT);
    ASSERT_TRUE(dis.valid());

    // subq $0x70, %rsp
    const char *code = "\x48\x83\xec\x70";
    auto res = dis.disassemble(QByteArray(code));
    ASSERT_NE(res, nullptr);
    EXPECT_EQ(res->count(), 1);

    auto *instr = res->instructions(0);
    ASSERT_NE(instr, nullptr);
    EXPECT_EQ(std::string(instr->mnemonic), std::string("subq")) << instr->mnemonic;
    EXPECT_EQ(std::string(instr->op_str), std::string("$0x70, %rsp")) << instr->op_str;
  }

  {
    auto obj = std::make_shared<BinaryObject>();
    Disassembler dis(obj);
    ASSERT_TRUE(dis.valid());

    auto res = dis.disassemble(QByteArray(nullptr));
    ASSERT_EQ(res, nullptr);
  }
}

TEST(Disassembler, disassembleText)
{
  {
    auto obj = std::make_shared<BinaryObject>();
    Disassembler dis(obj);
    ASSERT_TRUE(dis.valid());

    auto res = dis.disassemble(QString("90 90 90"));
    ASSERT_NE(res, nullptr);
    ASSERT_EQ(res->count(), 3);

    for (int i = 0; i < res->count(); i++) {
      auto *instr = res->instructions(i);
      ASSERT_NE(instr, nullptr);
      EXPECT_EQ(std::string(instr->mnemonic), std::string("nop")) << instr->mnemonic;
    }
  }
}
