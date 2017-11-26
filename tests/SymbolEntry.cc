#include "gtest/gtest.h"

#include "SymbolEntry.h"
using namespace dispar;

TEST(SymbolEntry, instantiate)
{
  SymbolEntry se(0, 0);
  SymbolEntry se2(0, 0, "");
}

TEST(SymbolEntry, index)
{
  SymbolEntry se(42, 0);
  EXPECT_EQ(se.index(), 42);
}

TEST(SymbolEntry, value)
{
  SymbolEntry se(0, 42);
  EXPECT_EQ(se.value(), 42);

  se.setValue(82);
  EXPECT_EQ(se.value(), 82);
}

TEST(SymbolEntry, string)
{
  SymbolEntry se(0, 0, "string");
  EXPECT_EQ(se.string(), "string");

  se.setString("ok");
  EXPECT_EQ(se.string(), "ok");
}

TEST(SymbolEntry, operatorEquals)
{
  {
    SymbolEntry se(42, 84, "hello");
    SymbolEntry se2(42, 84, "hello");
    EXPECT_EQ(se, se2);
  }

  {
    SymbolEntry se(42, 84, "hello");
    SymbolEntry se2(84, 42, "hello");
    EXPECT_NE(se, se2);
  }

  {
    SymbolEntry se(42, 84, "hello");
    SymbolEntry se2(42, 84, "no no");
    EXPECT_NE(se, se2);
  }
}

TEST(SymbolEntry, qHash)
{
  {
    SymbolEntry se(42, 84, "hello");
    SymbolEntry se2(42, 84, "hello");
    EXPECT_EQ(qHash(se), qHash(se2));
  }

  {
    SymbolEntry se(43, 84, "hello");
    SymbolEntry se2(42, 84, "hello");
    EXPECT_NE(qHash(se), qHash(se2));
  }

  {
    SymbolEntry se(42, 84, "hello");
    SymbolEntry se2(42, 804, "hello");
    EXPECT_NE(qHash(se), qHash(se2));
  }

  {
    SymbolEntry se(42, 84);
    SymbolEntry se2(42, 84, "hello");
    EXPECT_NE(qHash(se), qHash(se2));
  }
}
