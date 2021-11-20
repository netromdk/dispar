#include "gtest/gtest.h"

#include "BinaryObject.h"
#include "Constants.h"
using namespace dispar;

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
  EXPECT_TRUE(b.endianness() == Constants::Endianness::Little);

  b.setEndianness(Constants::Endianness::Big);
  EXPECT_TRUE(b.endianness() == Constants::Endianness::Big);
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
  EXPECT_EQ(sec2->address(), static_cast<quint64>(0x1));
  EXPECT_EQ(sec2->size(), static_cast<quint64>(1));
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
  EXPECT_EQ(sec2->address(), static_cast<quint64>(0x1));
  EXPECT_EQ(sec2->size(), static_cast<quint64>(1));
}

TEST(BinaryObject, sectionsByTypes)
{
  const QList<Section::Type> types{
    {Section::Type::TEXT, Section::Type::CODE_SIG, Section::Type::SYMBOLS}};

  BinaryObject b;
  EXPECT_EQ(0, b.sectionsByTypes(types).size());

  auto sec = std::make_unique<Section>(Section::Type::TEXT, "name", 0x1, 1);
  b.addSection(std::move(sec));

  auto sec2 = std::make_unique<Section>(Section::Type::CODE_SIG, "name2", 0x1, 1);
  b.addSection(std::move(sec2));

  auto sec3 = std::make_unique<Section>(Section::Type::SYMBOLS, "name3", 0x1, 1);
  b.addSection(std::move(sec3));

  const auto secs = b.sectionsByTypes(types);
  ASSERT_EQ(secs.size(), 3);

  auto *s = secs[0];
  EXPECT_EQ(s->type(), Section::Type::TEXT);
  EXPECT_EQ(s->name(), "name");
  EXPECT_EQ(s->address(), static_cast<quint64>(0x1));
  EXPECT_EQ(s->size(), static_cast<quint64>(1));
  s = secs[1];
  EXPECT_EQ(s->type(), Section::Type::CODE_SIG);
  EXPECT_EQ(s->name(), "name2");
  EXPECT_EQ(s->address(), static_cast<quint64>(0x1));
  EXPECT_EQ(s->size(), static_cast<quint64>(1));
  s = secs[2];
  EXPECT_EQ(s->type(), Section::Type::SYMBOLS);
  EXPECT_EQ(s->name(), "name3");
  EXPECT_EQ(s->address(), static_cast<quint64>(0x1));
  EXPECT_EQ(s->size(), static_cast<quint64>(1));
}

TEST(BinaryObject, symbolTable)
{
  SymbolTable st;
  BinaryObject b;
  b.setSymbolTable(st);
  EXPECT_EQ(b.symbolTable(), st);
}

TEST(BinaryObject, symbolTableMove)
{
  SymbolTable st;
  const auto copy = st;
  BinaryObject b;
  b.setSymbolTable(std::move(st));
  EXPECT_EQ(b.symbolTable(), copy);
}

TEST(BinaryObject, dynSymbolTable)
{
  SymbolTable st;
  BinaryObject b;
  b.setDynSymbolTable(st);
  EXPECT_EQ(b.dynSymbolTable(), st);
}

TEST(BinaryObject, dynSymbolTableMove)
{
  SymbolTable st;
  const auto copy = st;
  BinaryObject b;
  b.setDynSymbolTable(std::move(st));
  EXPECT_EQ(b.dynSymbolTable(), copy);
}

TEST(BinaryObject, cpuTypeX8664SystemBits64)
{
  BinaryObject b(CpuType::X86_64);
  EXPECT_EQ(b.cpuType(), CpuType::X86_64);
  EXPECT_EQ(b.systemBits(), 64);
}
