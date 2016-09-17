#include "gtest/gtest.h"

#include "Section.h"

TEST(Section, instantiate)
{
  Section s(Section::Type::Text, "test", 0x1, 1);
  Section s2(Section::Type::Text, "test", 0x1, 1, 100);
}

TEST(Section, types)
{
  EXPECT_EQ((int) Section::Type::Text, 0);
  EXPECT_EQ((int) Section::Type::SymbolStubs, 1);
  EXPECT_EQ((int) Section::Type::Symbols, 2);
  EXPECT_EQ((int) Section::Type::DynSymbols, 3);
  EXPECT_EQ((int) Section::Type::CString, 4);
  EXPECT_EQ((int) Section::Type::String, 5);
  EXPECT_EQ((int) Section::Type::FuncStarts, 6);
  EXPECT_EQ((int) Section::Type::CodeSig, 7);
}

TEST(Section, typeNames)
{
  EXPECT_EQ(Section::typeName(Section::Type::Text), "Text");
  EXPECT_EQ(Section::typeName(Section::Type::SymbolStubs), "Symbol Stubs");
  EXPECT_EQ(Section::typeName(Section::Type::Symbols), "Symbols");
  EXPECT_EQ(Section::typeName(Section::Type::DynSymbols), "Dynamic Symbols");
  EXPECT_EQ(Section::typeName(Section::Type::CString), "CString");
  EXPECT_EQ(Section::typeName(Section::Type::String), "String");
  EXPECT_EQ(Section::typeName(Section::Type::FuncStarts), "Function Starts");
  EXPECT_EQ(Section::typeName(Section::Type::CodeSig), "Code Signatures");
}

TEST(Section, type)
{
  Section s(Section::Type::Text, "test", 0x1, 1);
  EXPECT_EQ(s.type(), Section::Type::Text);
}

TEST(Section, name)
{
  Section s(Section::Type::Text, "test", 0x1, 1);
  EXPECT_EQ(s.name(), "test");
}

TEST(Section, address)
{
  Section s(Section::Type::Text, "test", 0x1, 1);
  EXPECT_EQ(s.address(), 0x1);
}

TEST(Section, size)
{
  Section s(Section::Type::Text, "test", 0x1, 1);
  EXPECT_EQ(s.size(), 1);
}

TEST(Section, offset)
{
  Section s(Section::Type::Text, "test", 0x1, 1, 10);
  EXPECT_EQ(s.offset(), 10);
}

TEST(Section, data)
{
  Section s(Section::Type::Text, "test", 0x1, 1);
  EXPECT_EQ(s.data().size(), 0);

  s.setData("hello");
  EXPECT_EQ(s.data(), "hello");
}

TEST(Section, isModified)
{
  Section s(Section::Type::Text, "test", 0x1, 1);
  EXPECT_FALSE(s.isModified());
}

TEST(Section, modifiedWhen)
{
  Section s(Section::Type::Text, "test", 0x1, 1);
  EXPECT_TRUE(s.modifiedWhen().isNull());
}

TEST(Section, modifiedRegions)
{
  Section s(Section::Type::Text, "test", 0x1, 1);
  EXPECT_TRUE(s.modifiedRegions().isEmpty());
}

TEST(Section, setSubData)
{
  {
    Section s(Section::Type::Text, "test", 0x1, 1);
    s.setData("ABCD");
    s.setSubData("X", -1); // fails
    s.setSubData("Y", 100); // fails
  }

  {
    Section s(Section::Type::Text, "test", 0x1, 1);
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
