#include "gtest/gtest.h"

#include "testutils.h"

#include "BinaryObject.h"
#include "Disassembler.h"
#include "Section.h"
using namespace dispar;

TEST(Section, instantiate)
{
  Section s(Section::Type::TEXT, "test", 0x1, 1);
  Section s2(Section::Type::TEXT, "test", 0x1, 1, 100);
}

TEST(Section, types)
{
  EXPECT_EQ((int) Section::Type::TEXT, 0);
  EXPECT_EQ((int) Section::Type::SYMBOL_STUBS, 1);
  EXPECT_EQ((int) Section::Type::SYMBOLS, 2);
  EXPECT_EQ((int) Section::Type::DYN_SYMBOLS, 3);
  EXPECT_EQ((int) Section::Type::CSTRING, 4);
  EXPECT_EQ((int) Section::Type::STRING, 5);
  EXPECT_EQ((int) Section::Type::FUNC_STARTS, 6);
  EXPECT_EQ((int) Section::Type::CODE_SIG, 7);
  EXPECT_EQ((int) Section::Type::LC_VERSION_MIN_MACOSX, 8);
  EXPECT_EQ((int) Section::Type::LC_VERSION_MIN_IPHONEOS, 9);
  EXPECT_EQ((int) Section::Type::LC_VERSION_MIN_WATCHOS, 10);
  EXPECT_EQ((int) Section::Type::LC_VERSION_MIN_TVOS, 11);
}

TEST(Section, typeNames)
{
  EXPECT_EQ(Section::typeName(Section::Type::TEXT), "Text");
  EXPECT_EQ(Section::typeName(Section::Type::SYMBOL_STUBS), "Symbol Stubs");
  EXPECT_EQ(Section::typeName(Section::Type::SYMBOLS), "Symbols");
  EXPECT_EQ(Section::typeName(Section::Type::DYN_SYMBOLS), "Dynamic Symbols");
  EXPECT_EQ(Section::typeName(Section::Type::CSTRING), "CString");
  EXPECT_EQ(Section::typeName(Section::Type::STRING), "String");
  EXPECT_EQ(Section::typeName(Section::Type::FUNC_STARTS), "Function Starts");
  EXPECT_EQ(Section::typeName(Section::Type::CODE_SIG), "Code Signatures");
  EXPECT_EQ(Section::typeName(Section::Type::LC_VERSION_MIN_MACOSX), "LC_VERSION_MIN_MACOSX");
  EXPECT_EQ(Section::typeName(Section::Type::LC_VERSION_MIN_IPHONEOS), "LC_VERSION_MIN_IPHONEOS");
  EXPECT_EQ(Section::typeName(Section::Type::LC_VERSION_MIN_WATCHOS), "LC_VERSION_MIN_WATCHOS");
  EXPECT_EQ(Section::typeName(Section::Type::LC_VERSION_MIN_TVOS), "LC_VERSION_MIN_TVOS");

  // Empty string for unknown type.
  EXPECT_EQ(Section::typeName(Section::Type(-1)), "");
}

TEST(Section, type)
{
  Section s(Section::Type::TEXT, "test", 0x1, 1);
  EXPECT_EQ(s.type(), Section::Type::TEXT);
}

TEST(Section, name)
{
  Section s(Section::Type::TEXT, "test", 0x1, 1);
  EXPECT_EQ(s.name(), "test");
}

TEST(Section, address)
{
  Section s(Section::Type::TEXT, "test", 0x1, 1);
  EXPECT_EQ(s.address(), static_cast<quint64>(0x1));
}

TEST(Section, size)
{
  Section s(Section::Type::TEXT, "test", 0x1, 1);
  EXPECT_EQ(s.size(), static_cast<quint64>(1));
}

TEST(Section, offset)
{
  Section s(Section::Type::TEXT, "test", 0x1, 1, 10);
  EXPECT_EQ(s.offset(), static_cast<quint32>(10));
}

TEST(Section, data)
{
  Section s(Section::Type::TEXT, "test", 0x1, 1);
  EXPECT_EQ(s.data().size(), 0);

  s.setData("hello");
  EXPECT_EQ(s.data(), "hello");
}

TEST(Section, isModified)
{
  Section s(Section::Type::TEXT, "test", 0x1, 1);
  EXPECT_FALSE(s.isModified());
}

TEST(Section, modifiedWhen)
{
  Section s(Section::Type::TEXT, "test", 0x1, 1);
  EXPECT_TRUE(s.modifiedWhen().isNull());
}

TEST(Section, modifiedRegions)
{
  Section s(Section::Type::TEXT, "test", 0x1, 1);
  EXPECT_TRUE(s.modifiedRegions().isEmpty());
}

TEST(Section, setSubData)
{
  {
    Section s(Section::Type::TEXT, "test", 0x1, 1);
    s.setData("ABCD");
    s.setSubData("X", -1);  // fails
    s.setSubData("Y", 100); // fails
  }

  {
    Section s(Section::Type::TEXT, "test", 0x1, 1);
    s.setData("ABCD");
    s.setSubData("X", 1);
    EXPECT_EQ(s.data(), "AXCD");
    EXPECT_TRUE(s.isModified());
    EXPECT_FALSE(s.modifiedWhen().isNull());

    const auto &pairs = s.modifiedRegions();
    ASSERT_EQ(pairs.size(), 1);
    auto pair = pairs[0];
    EXPECT_EQ(pair.position(), 1);
    EXPECT_EQ(pair.size(), 1);
  }

  {
    Section s(Section::Type::TEXT, "test", 0x1, 1);
    s.setData("ABCD");

    // These are all the same pos+size so only the last one is kept, "Z".
    s.setSubData("X", 1);
    s.setSubData("Y", 1);
    s.setSubData("X", 1);
    s.setSubData("Z", 1);

    const auto &pairs = s.modifiedRegions();
    ASSERT_EQ(pairs.size(), 1);
    EXPECT_EQ(pairs[0], Section::ModifiedRegion(1, "Z"));
  }

  {
    Section s(Section::Type::TEXT, "test", 0x1, 1);
    s.setData("ABCD");

    // First region is contained in the newer second region, so it is removed.
    s.setSubData("X", 2);
    s.setSubData("123", 1);

    const auto &pairs = s.modifiedRegions();
    ASSERT_EQ(pairs.size(), 1);
    EXPECT_EQ(pairs[0], Section::ModifiedRegion(1, "123"));
  }

  {
    Section s(Section::Type::TEXT, "test", 0x1, 1);
    s.setData("ABCD");

    // All regions are contained in newer regions, thus only the last is left.
    s.setSubData("X", 0);
    s.setSubData("YZ", 1);
    s.setSubData("H", 2);
    s.setSubData("qwe", 0);
    s.setSubData("123", 0);

    const auto &pairs = s.modifiedRegions();
    ASSERT_EQ(pairs.size(), 1);
    EXPECT_EQ(pairs[0], Section::ModifiedRegion(0, "123"));
  }

  {
    Section s(Section::Type::TEXT, "test", 0x1, 1);
    s.setData("ABCD");
    s.setSubData("1234", 0); // 1234
    s.setSubData("XY", 1);   // 1XY4
    s.setSubData("Z", 2);    // 1XZ4

    // All three are still left.
    ASSERT_EQ(s.modifiedRegions().size(), 3);

    s.setSubData("89", 1); // 1894

    // The middle two are removed.
    const auto &pairs = s.modifiedRegions();
    ASSERT_EQ(pairs.size(), 2);
    EXPECT_EQ(pairs[0], Section::ModifiedRegion(0, "1234"));
    EXPECT_EQ(pairs[1], Section::ModifiedRegion(1, "89"));
  }

  {
    Section s(Section::Type::TEXT, "test", 0x1, 1);
    s.setData("ABCDE");
    s.setSubData("123", 1); // A123E
    s.setSubData("QWE", 2); // A1QWE

    // All are still left.
    ASSERT_EQ(s.modifiedRegions().size(), 2);

    // No regions are contained yet, though the first is eclipsed by the newest two.
    s.setSubData("IOP", 0); // IOPWE
    ASSERT_EQ(s.modifiedRegions().size(), 3);

    // All are contained in the last region.
    s.setSubData("ABCDE", 0); // ABCDE
    const auto &pairs = s.modifiedRegions();
    ASSERT_EQ(pairs.size(), 1);
    EXPECT_EQ(pairs[0], Section::ModifiedRegion(0, "ABCDE"));
  }
}

TEST(Section, hasAddress)
{
  Section s(Section::Type::TEXT, "test", 1, 10, 10);
  EXPECT_FALSE(s.hasAddress(0));
  EXPECT_TRUE(s.hasAddress(1));
  EXPECT_TRUE(s.hasAddress(9));
  EXPECT_TRUE(s.hasAddress(10));
  EXPECT_FALSE(s.hasAddress(11));
}

TEST(Section, toString)
{
  const Section s(Section::Type::TEXT, "test", 1, 10, 10);
  const auto expect = QString("%1 (%2)").arg(s.name()).arg(Section::typeName(s.type()));
  const auto str = s.toString();
  EXPECT_EQ(expect, str) << str;
}

TEST(Section, setGetDisassembly)
{
  auto obj = std::make_unique<BinaryObject>();
  obj->setCpuType(CpuType::X86_64);

  Disassembler disasm(*obj.get());
  ASSERT_TRUE(disasm.valid());

  auto res = disasm.disassemble(QString("90 90 90"));
  ASSERT_NE(nullptr, res);
  const auto count = res->count();
  EXPECT_EQ((unsigned long) 3, count);

  Section s(Section::Type::TEXT, "test", 1, 10, 10);
  EXPECT_EQ(nullptr, s.disassembly());

  s.setDisassembly(std::move(res));
  ASSERT_NE(nullptr, s.disassembly());
  EXPECT_EQ(count, s.disassembly()->count());
}

TEST(SectionModifiedRegion, operatorEquals)
{
  const Section::ModifiedRegion r{0, "x"}, r2{20, "xyz"}, r3{0, "y"};
  EXPECT_EQ(r, r);
  EXPECT_NE(r, r2);
  EXPECT_NE(r2, r);
  EXPECT_NE(r, r3);
  EXPECT_NE(r3, r);
  EXPECT_NE(r2, r3);
  EXPECT_NE(r3, r2);
}
