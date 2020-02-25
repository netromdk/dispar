#include "gtest/gtest.h"

#include "testutils.h"

#include <QBuffer>

#include "BinaryObject.h"
#include "formats/Format.h"
#include "formats/MachO.h"
using namespace dispar;

TEST(Format, types)
{
  EXPECT_EQ((int) Format::Type::MACH_O, 0);
}

TEST(Format, typeNames)
{
  EXPECT_EQ(Format::typeName(Format::Type::MACH_O), "Mach-O");
  EXPECT_EQ(Format::typeName(static_cast<Format::Type>(-1)), "");
}

TEST(Format, detect)
{
  auto fmt = Format::detect("");
  ASSERT_EQ(fmt, nullptr);

  fmt = Format::detect(":macho_main");
  ASSERT_NE(fmt, nullptr);
  EXPECT_EQ(fmt->type(), Format::Type::MACH_O);
}

TEST(Format, write)
{
  auto format = std::make_shared<MachO>(":macho_main");
  ASSERT_TRUE(format->parse());

  QFile f(format->file());
  ASSERT_TRUE(f.open(QIODevice::ReadOnly));
  auto data = f.readAll();
  f.close();

  auto objects = format->objects();
  ASSERT_EQ(1, objects.size());

  auto sections = objects.first()->sections();
  ASSERT_GT(sections.size(), 1);

  auto *section = sections.first();
  ASSERT_NE(nullptr, section);

  const QByteArray block("abcd");
  EXPECT_EQ(QByteArray("UH\x89\xE5"), data.mid(section->offset(), block.size()));

  section->setSubData(block, 0);

  QBuffer buf(&data);
  buf.open(QIODevice::ReadWrite);
  format->write(buf);

  EXPECT_EQ(block, data.mid(section->offset(), block.size()));
}
