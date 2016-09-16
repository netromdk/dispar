#include "gtest/gtest.h"

#include "Reader.h"

#include <QBuffer>

#include <memory>

class ReaderTest : public ::testing::Test {
public:
  void SetUp() override
  {
    buffer = std::make_unique<QBuffer>(&array);
    buffer->open(QIODevice::ReadOnly);

    reader = std::make_unique<Reader>(*buffer.get());
  }

  QByteArray array;
  std::unique_ptr<QBuffer> buffer;
  std::unique_ptr<Reader> reader;
};

TEST(Reader, instantiate)
{
  QBuffer buf;
  Reader reader(buf);
  EXPECT_TRUE(reader.isLittleEndian());
}

TEST_F(ReaderTest, read)
{
  QByteArray tmpArray(64, 'X');
  array.append(tmpArray);

  ASSERT_EQ(reader->pos(), 0);

  auto tmp = reader->read(tmpArray.size());
  EXPECT_EQ(tmp, tmpArray);

  ASSERT_EQ(reader->pos(), tmpArray.size());
}

TEST_F(ReaderTest, seekPos)
{
  QByteArray tmpArray(64, 'X');
  array.append(tmpArray);

  ASSERT_EQ(reader->pos(), 0);

  ASSERT_TRUE(reader->seek(10));
  EXPECT_EQ(reader->pos(), 10);

  ASSERT_TRUE(reader->seek(0));
  EXPECT_EQ(reader->pos(), 0);
}

TEST_F(ReaderTest, atEnd)
{
  QByteArray tmpArray(64, 'X');
  array.append(tmpArray);

  ASSERT_FALSE(reader->atEnd());
  ASSERT_EQ(reader->pos(), 0);

  ASSERT_TRUE(reader->seek(tmpArray.size()));
  EXPECT_EQ(reader->pos(), tmpArray.size());
  EXPECT_TRUE(reader->atEnd());
}

TEST_F(ReaderTest, getChar)
{
  auto ch = 42;
  array.append(ch);

  ASSERT_EQ(reader->pos(), 0);

  bool ok;
  auto tmp = reader->getChar(&ok);
  EXPECT_TRUE(ok);
  EXPECT_EQ(tmp, ch);

  EXPECT_EQ(reader->pos(), 1);
}

TEST_F(ReaderTest, getUChar)
{
  auto ch = 42;
  array.append(ch);

  ASSERT_EQ(reader->pos(), 0);

  bool ok;
  auto tmp = reader->getUChar(&ok);
  EXPECT_TRUE(ok);
  EXPECT_EQ(tmp, ch);

  EXPECT_EQ(reader->pos(), 1);
}

TEST_F(ReaderTest, peekChar)
{
  auto ch = 42;
  array.append(ch);

  ASSERT_EQ(reader->pos(), 0);

  bool ok;
  auto tmp = reader->peekChar(&ok);
  EXPECT_TRUE(ok);
  EXPECT_EQ(tmp, ch);

  EXPECT_EQ(reader->pos(), 0);
}

TEST_F(ReaderTest, peekUChar)
{
  auto ch = 42;
  array.append(ch);

  ASSERT_EQ(reader->pos(), 0);

  bool ok;
  auto tmp = reader->peekUChar(&ok);
  EXPECT_TRUE(ok);
  EXPECT_EQ(tmp, ch);

  EXPECT_EQ(reader->pos(), 0);
}

TEST_F(ReaderTest, peekList)
{
  EXPECT_FALSE(reader->peekList({}));

  std::initializer_list<unsigned char> list = {1, 2, 3, 4};

  for (const auto &ch : list) {
    array.append(ch);
  }

  ASSERT_EQ(reader->pos(), 0);
  EXPECT_TRUE(reader->peekList(list));
  EXPECT_EQ(reader->pos(), 0);

  array.clear();
  EXPECT_FALSE(reader->peekList(list));

  array.append(1);
  EXPECT_FALSE(reader->peekList({2}));
}

TEST_F(ReaderTest, getUInt16)
{
  quint16 val = 513; // = 1 + 2*8

  array.append(1);
  array.append(2);

  bool ok;
  auto tmp = reader->getUInt16(&ok);
  EXPECT_TRUE(ok);
  EXPECT_EQ(tmp, val);

  array.clear();
  tmp = reader->getUInt16(&ok);
  EXPECT_FALSE(ok);
}

TEST_F(ReaderTest, getUInt16BigEndian)
{
  quint16 val = 513; // = 1 + 2*8

  array.append(2);
  array.append(1);

  reader->setLittleEndian(false);

  bool ok;
  auto tmp = reader->getUInt16(&ok);
  EXPECT_TRUE(ok);
  EXPECT_EQ(tmp, val);

  array.clear();
  tmp = reader->getUInt16(&ok);
  EXPECT_FALSE(ok);
}

TEST_F(ReaderTest, getUInt32)
{
  quint32 val = 67305985; // = 1 + 2*8 + 3<<(2*8) + 4<<(3*8)

  array.append(1);
  array.append(2);
  array.append(3);
  array.append(4);

  bool ok;
  auto tmp = reader->getUInt32(&ok);
  EXPECT_TRUE(ok);
  EXPECT_EQ(tmp, val);

  array.clear();
  tmp = reader->getUInt32(&ok);
  EXPECT_FALSE(ok);
}

TEST_F(ReaderTest, getUInt32BigEndian)
{
  quint32 val = 67305985; // = 1 + 2*8 + 3<<(2*8) + 4<<(3*8)

  array.append(4);
  array.append(3);
  array.append(2);
  array.append(1);

  reader->setLittleEndian(false);

  bool ok;
  auto tmp = reader->getUInt32(&ok);
  EXPECT_TRUE(ok);
  EXPECT_EQ(tmp, val);

  array.clear();
  tmp = reader->getUInt32(&ok);
  EXPECT_FALSE(ok);
}

TEST_F(ReaderTest, getUInt64)
{
  // 1 + 2*8 + 3<<(2*8) + 4<<(3*8) + 1<<(4*8) + 2<<(5*8) + 3<<(6*8) + 4<<(7*8)
  quint64 val = 289077004467372545;

  array.append(1);
  array.append(2);
  array.append(3);
  array.append(4);
  array.append(1);
  array.append(2);
  array.append(3);
  array.append(4);

  bool ok;
  auto tmp = reader->getUInt64(&ok);
  EXPECT_TRUE(ok);
  EXPECT_EQ(tmp, val);

  array.clear();
  tmp = reader->getUInt64(&ok);
  EXPECT_FALSE(ok);
}

TEST_F(ReaderTest, getUInt64BigEndian)
{
  // 1 + 2*8 + 3<<(2*8) + 4<<(3*8) + 1<<(4*8) + 2<<(5*8) + 3<<(6*8) + 4<<(7*8)
  quint64 val = 289077004467372545;

  array.append(4);
  array.append(3);
  array.append(2);
  array.append(1);
  array.append(4);
  array.append(3);
  array.append(2);
  array.append(1);

  reader->setLittleEndian(false);

  bool ok;
  auto tmp = reader->getUInt64(&ok);
  EXPECT_TRUE(ok);
  EXPECT_EQ(tmp, val);

  array.clear();
  tmp = reader->getUInt64(&ok);
  EXPECT_FALSE(ok);
}

int main(int argc, char **argv)
{
  testing::InitGoogleTest(&argc, argv);
  return (RUN_ALL_TESTS());
}
