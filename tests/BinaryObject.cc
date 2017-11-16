#include "gtest/gtest.h"

#include "BinaryObject.h"

TEST(BinaryObject, instantiate)
{
  BinaryObject b;
  BinaryObject b2(CpuType::X86_64);
}

TEST(BinaryObject, cpuType)
{
  BinaryObject b(CpuType::X86_64);
  EXPECT_EQ(b.cpuType(), CpuType::X86_64);

  b.setCpuType(CpuType::ARM);
  EXPECT_EQ(b.cpuType(), CpuType::ARM);
}

TEST(BinaryObject, cpuSubType)
{
  BinaryObject b(CpuType::X86_64, CpuType::CELERON);
  EXPECT_EQ(b.cpuSubType(), CpuType::CELERON);

  b.setCpuSubType(CpuType::XEON);
  EXPECT_EQ(b.cpuSubType(), CpuType::XEON);
}

TEST(BinaryObject, isLittleEndian)
{
  BinaryObject b;
  EXPECT_TRUE(b.isLittleEndian());

  b.setLittleEndian(false);
  EXPECT_FALSE(b.isLittleEndian());
}

TEST(BinaryObject, systemBits)
{
  BinaryObject b;
  EXPECT_EQ(b.systemBits(), 32);

  b.setSystemBits(64);
  EXPECT_EQ(b.systemBits(), 64);
}

TEST(BinaryObject, fileType)
{
  BinaryObject b;
  EXPECT_EQ(b.fileType(), FileType::EXECUTE);

  b.setFileType(FileType::OBJECT);
  EXPECT_EQ(b.fileType(), FileType::OBJECT);
}

TEST(BinaryObject, section)
{
  BinaryObject b;
  EXPECT_EQ(b.section(Section::Type::TEXT), nullptr);

  auto sec = std::make_unique<Section>(Section::Type::TEXT, "name", 0x1, 1);
  b.addSection(std::move(sec));

  auto *sec2 = b.section(Section::Type::TEXT);
  EXPECT_EQ(sec2->type(), Section::Type::TEXT);
  EXPECT_EQ(sec2->name(), "name");
  EXPECT_EQ(sec2->address(), 0x1);
  EXPECT_EQ(sec2->size(), 1);
}

TEST(BinaryObject, sectionsByType)
{
  BinaryObject b;
  EXPECT_EQ(b.sectionsByType(Section::Type::TEXT).size(), 0);

  auto sec = std::make_unique<Section>(Section::Type::TEXT, "name", 0x1, 1);
  b.addSection(std::move(sec));

  auto secs = b.sectionsByType(Section::Type::TEXT);
  ASSERT_EQ(secs.size(), 1);

  auto *sec2 = secs[0];
  EXPECT_EQ(sec2->type(), Section::Type::TEXT);
  EXPECT_EQ(sec2->name(), "name");
  EXPECT_EQ(sec2->address(), 0x1);
  EXPECT_EQ(sec2->size(), 1);
}

TEST(BinaryObject, symbolTable)
{
  SymbolTable st;
  BinaryObject b;
  b.setSymbolTable(st);
  EXPECT_EQ(b.symbolTable(), st);
}

TEST(BinaryObject, dynSymbolTable)
{
  SymbolTable st;
  BinaryObject b;
  b.setDynSymbolTable(st);
  EXPECT_EQ(b.dynSymbolTable(), st);
}

TEST(BinaryObject, cpuTypeX8664SystemBits64)
{
  BinaryObject b(CpuType::X86_64);
  EXPECT_EQ(b.cpuType(), CpuType::X86_64);
  EXPECT_EQ(b.systemBits(), 64);
}
