#include "testutils.h"
#include "gtest/gtest.h"

#include "Debugger.h"
using namespace dispar;

TEST(Debugger, instantiate)
{
  Debugger dbg;
  Debugger dbg2("lldb", "--version", "-- {{BINARY}} {{ARGS}}");
}

TEST(Debugger, valid)
{
  EXPECT_FALSE(Debugger().valid());
  EXPECT_FALSE(Debugger("", "", "").valid());
  EXPECT_FALSE(Debugger("", "--version", "").valid());
  EXPECT_FALSE(Debugger("lldb", "", "").valid());
  EXPECT_FALSE(Debugger("lldb", "", "-- {{BINARY}} {{ARGS}}").valid());
  EXPECT_FALSE(Debugger("", "", "-- {{BINARY}} {{ARGS}}").valid());
  EXPECT_FALSE(Debugger("", "--version", "-- {{BINARY}} {{ARGS}}").valid());
  EXPECT_TRUE(Debugger("lldb", "--version", "-- {{BINARY}} {{ARGS}}").valid());
}

TEST(Debugger, substituteLaunchPattern)
{
  Debugger dbg("lldb", "--version", "-- {{BINARY}} {{ARGS}}");
  const QString binary("hello");
  const QStringList args({"one", "two", "three"});
  EXPECT_EQ(dbg.substituteLaunchPattern(binary), QString("-- %1 ").arg(binary));
  EXPECT_EQ(dbg.substituteLaunchPattern(binary, args),
            QString("-- %1 %2").arg(binary).arg(args.join(" ")));
}

TEST(Debugger, runnableTimeout)
{
  Debugger dbg("somethingirrelevant", "--blarg", "-- {{BINARY}} {{ARGS}}");
  EXPECT_FALSE(dbg.runnable(1));
}

TEST(Debugger, predefined)
{
  const QList<Debugger> predef{{"lldb", "--version", "-- {{BINARY}} {{ARGS}}"},
                               {"gdb", "--version", "--args {{BINARY}} {{ARGS}}"},
                               {"ggdb", "--version", "--args {{BINARY}} {{ARGS}}"}};
  EXPECT_EQ(predef, Debugger::predefined());
}

TEST(Debugger, toString)
{
  const QString dbg("lldb"), version("--version"), pattern("-- {{BINARY}} {{ARGS}}");
  const auto expect = QString("Debugger[program = \"%1\", version = \"%2\", launch = \"%3\"]")
                        .arg(dbg)
                        .arg(version)
                        .arg(pattern);
  const auto str = Debugger{dbg, version, pattern}.toString();
  EXPECT_EQ(expect, str) << expect;
}

TEST(Debugger, operatorEqual)
{
  const Debugger dbg1{"lldb", "--version", "-- {{BINARY}} {{ARGS}}"};
  EXPECT_EQ(dbg1, dbg1);
}

TEST(Debugger, operatorNotEqual)
{
  const Debugger dbg1{"lldb", "--version", "-- {{BINARY}} {{ARGS}}"};
  const Debugger dbg2{"gdb", "--version", "--args {{BINARY}} {{ARGS}}"};
  EXPECT_NE(dbg1, dbg2);
  EXPECT_NE(dbg2, dbg1);
}

TEST(Debugger, qDebugOperator)
{
  const Debugger dbg{"lldb", "--version", "-- {{BINARY}} {{ARGS}}"};
  QString output;
  QDebug qdbg(&output);
  qdbg.nospace().noquote() << dbg;
  EXPECT_EQ(output, dbg.toString());
}
