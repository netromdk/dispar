#include "gtest/gtest.h"

#include "CStringReader.h"

TEST(CStringReader, instantiate)
{
  QByteArray data;
  CStringReader reader(data);
}

TEST(CStringReader, next)
{
  {
    QByteArray data("hello");
    CStringReader reader(data);
    ASSERT_TRUE(reader.next());
    EXPECT_EQ(reader.string(), "hello");
    ASSERT_FALSE(reader.next());
  }

  {
    QByteArray data;
    data += "one";
    data += (char) 0;
    data += "two";
    data += (char) 0;
    data += "three";
    data += (char) 0;

    // TODO: implement ostream << QString in testutils
    CStringReader reader(data);
    ASSERT_TRUE(reader.next());
    EXPECT_EQ(reader.string(), "one") << reader.string().toUtf8().constData();
    ASSERT_TRUE(reader.next());
    EXPECT_EQ(reader.string(), "two") << reader.string().toUtf8().constData();
    ASSERT_TRUE(reader.next());
    EXPECT_EQ(reader.string(), "three") << reader.string().toUtf8().constData();
    ASSERT_FALSE(reader.next());
  }
}

TEST(CStringReader, string)
{
  QByteArray data;
  CStringReader reader(data);
  EXPECT_EQ(reader.string(), QString());
}

TEST(CStringReader, readAll)
{
  {
    QByteArray data("hello");
    CStringReader reader(data);
    auto strings = reader.readAll();
    ASSERT_EQ(strings.size(), 1);
    EXPECT_EQ(strings[0], "hello");
  }

  {
    QByteArray data;
    data += "one";
    data += (char) 0;
    data += "two";
    data += (char) 0;
    data += "three";
    data += (char) 0;

    // TODO: implement ostream << QString in testutils
    CStringReader reader(data);
    auto strings = reader.readAll();
    ASSERT_EQ(strings.size(), 3);
    EXPECT_EQ(strings[0], "one");
    EXPECT_EQ(strings[1], "two");
    EXPECT_EQ(strings[2], "three");
  }
}
