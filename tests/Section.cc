#include "gtest/gtest.h"

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
    s.setSubData("X", -1); // fails
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
    EXPECT_EQ(pair.first, 1);
    EXPECT_EQ(pair.second, 1);
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
