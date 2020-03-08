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

TEST(Format, toString)
{
  auto format = std::make_shared<MachO>(":macho_main_32_64");
  ASSERT_TRUE(format->parse());

  const auto str = format->toString();
  EXPECT_EQ(R"***(Format: Mach-O
2 objects:
- x86 (i386), 32-bit, Executable, Little Endian, 5 sections
  - Program (Text)
      0x1fa0-0x1fb2 (18.0 B)
  - String Table (String)
      0x1054-0x1084 (48.0 B)
      0x2054 offset
  - macOS SDK min version (LC_VERSION_MIN_MACOSX)
      0x1220-0x1228 (8.0 B)
  - Function Starts (Function Starts)
      0x102c-0x1030 (4.0 B)
      0x202c offset
  - Symbol Table (Symbols)
      0x1030-0x1054 (36.0 B)
      0x2030 offset
- x86 64 (i386), 64-bit, Executable, Little Endian, 5 sections
  - Program (Text)
      0x100000fa0-0x100000faf (15.0 B)
      0x3fa0 offset
  - String Table (String)
      0x1068-0x1098 (48.0 B)
      0x4068 offset
  - macOS SDK min version (LC_VERSION_MIN_MACOSX)
      0x3270-0x3278 (8.0 B)
  - Function Starts (Function Starts)
      0x1030-0x1038 (8.0 B)
      0x4030 offset
  - Symbol Table (Symbols)
      0x1038-0x1068 (48.0 B)
      0x4038 offset
)***",
            str)
    << str;
}
