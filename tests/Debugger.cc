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
