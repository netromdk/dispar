#include "gtest/gtest.h"

#include "SymbolTable.h"
using namespace dispar;

TEST(SymbolTable, instantiate)
{
  SymbolTable st;
}

TEST(SymbolTable, symbols)
{
  SymbolEntry sym(1, 2);

  SymbolTable st;
  st.addSymbol(sym);

  // QList<SymbolEntry> &symbols()
  const auto &syms = st.symbols();
  ASSERT_EQ(syms.size(), 1);
  EXPECT_EQ(syms[0], sym);

  // const QList<SymbolEntry> &symbols() const
  class A {
  public:
    static void test(const SymbolTable &table)
    {
      const auto &syms2 = table.symbols();
      ASSERT_EQ(syms2.size(), 1);
    }
  };
  A::test(st);
}

TEST(SymbolTable, string)
{
  SymbolEntry sym(1, 2, "hello");
  SymbolEntry sym2(3, 4);

  SymbolTable st;
  st.addSymbol(sym);
  st.addSymbol(sym2);

  QString str;
  EXPECT_TRUE(st.string(sym.value(), str));
  EXPECT_EQ(str, sym.string());

  str.clear();
  EXPECT_FALSE(st.string(1234, str));
  EXPECT_TRUE(str.isEmpty());

  EXPECT_FALSE(st.string(sym2.value(), str));
  EXPECT_TRUE(str.isEmpty());
}

TEST(SymbolTable, operatorEquals)
{
  {
    SymbolEntry se(42, 84, "hello");
    SymbolEntry se2(42, 84, "hello");

    SymbolTable st;
    st.addSymbol(se);
    st.addSymbol(se2);

    SymbolTable st2;
    st2.addSymbol(se);
    st2.addSymbol(se2);

    EXPECT_EQ(st, st2);
  }

  {
    SymbolEntry se(42, 84, "hello");
    SymbolEntry se2(42, 84, "hello");

    SymbolTable st;
    st.addSymbol(se);
    st.addSymbol(se2);

    SymbolTable st2;
    st2.addSymbol(se2);

    EXPECT_NE(st, st2);
  }
}
