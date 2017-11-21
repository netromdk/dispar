#include "gtest/gtest.h"

#include "testutils.h"

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

    CStringReader reader(data);
    ASSERT_TRUE(reader.next());
    EXPECT_EQ(reader.string(), "one") << reader.string();
    EXPECT_EQ(reader.offset(), 0) << reader.offset();

    ASSERT_TRUE(reader.next());
    EXPECT_EQ(reader.string(), "two") << reader.string();
    EXPECT_EQ(reader.offset(), 4) << reader.offset();

    ASSERT_TRUE(reader.next());
    EXPECT_EQ(reader.string(), "three") << reader.string();
    EXPECT_EQ(reader.offset(), 8) << reader.offset();

    ASSERT_FALSE(reader.next());
  }
}

TEST(CStringReader, string)
{
  QByteArray data;
  CStringReader reader(data);
  EXPECT_EQ(reader.string(), QString());
}

TEST(CStringReader, offset)
{
  QByteArray data;
  CStringReader reader(data);
  EXPECT_EQ(reader.offset(), 0);
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

    CStringReader reader(data);
    auto strings = reader.readAll();
    ASSERT_EQ(strings.size(), 3);
    EXPECT_EQ(strings[0], "one");
    EXPECT_EQ(strings[1], "two");
    EXPECT_EQ(strings[2], "three");
  }
}

TEST(CStringReader, dataWithConsecutiveNullBytes)
{
  QFile f(":strings.bin");
  ASSERT_TRUE(f.open(QIODevice::ReadOnly));
  const auto data = f.readAll();
  f.close();

  {
    CStringReader reader(data);
    ASSERT_TRUE(reader.next());
    EXPECT_EQ(reader.string(), "_InternalFlxGetInitStatus");
    ASSERT_TRUE(reader.next());
    EXPECT_EQ(reader.string(), "_Java_com_macrovision_flexlm_HostId_lGetNativeHostId");
    ASSERT_TRUE(reader.next());
    EXPECT_EQ(reader.string(), "_NSGLGetProcAddress");
    ASSERT_TRUE(reader.next());
    EXPECT_EQ(reader.string(), "_Ox3496");
    ASSERT_FALSE(reader.next());
  }

  {
    CStringReader reader(data);
    const auto strings = reader.readAll();
    ASSERT_EQ(strings.size(), 4);
    EXPECT_EQ(strings[0], "_InternalFlxGetInitStatus");
    EXPECT_EQ(strings[1], "_Java_com_macrovision_flexlm_HostId_lGetNativeHostId");
    EXPECT_EQ(strings[2], "_NSGLGetProcAddress");
    EXPECT_EQ(strings[3], "_Ox3496");
  }
}
