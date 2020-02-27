#include "gtest/gtest.h"

#include "testutils.h"

#include "BinaryObject.h"
#include "CStringReader.h"
#include "formats/MachO.h"
using namespace dispar;

TEST(MachO, instantiate)
{
  MachO fmt("some_file");
  EXPECT_EQ(fmt.type(), Format::Type::MACH_O);
}

TEST(MachO, file)
{
  QString file("test");
  MachO fmt(file);
  EXPECT_EQ(fmt.file(), file);
}

TEST(MachO, detect)
{
  {
    MachO fmt("_/something_that_does_not_exist");
    EXPECT_FALSE(fmt.detect());
  }

  {
    auto file = tempFile();

    MachO fmt(file->fileName());
    EXPECT_FALSE(fmt.detect());
  }

  {
    // Something less than 4 bytes.
    auto file = tempFile("h");

    MachO fmt(file->fileName());
    EXPECT_FALSE(fmt.detect());
  }

  {
    // Something more than 4 bytes.
    auto file = tempFile("hello there, man");

    MachO fmt(file->fileName());
    EXPECT_FALSE(fmt.detect());
  }

  {
    MachO fmt(":macho_main");
    EXPECT_TRUE(fmt.detect());
  }

  {
    MachO fmt(":macho_main_32");
    EXPECT_TRUE(fmt.detect());
  }
}

TEST(MachO, objects)
{
  {
    MachO fmt("");
    EXPECT_EQ(fmt.objects().size(), 0);
  }

  {
    MachO fmt(":macho_main");
    EXPECT_TRUE(fmt.parse());
    EXPECT_EQ(fmt.objects().size(), 1);
  }
}

TEST(MachO, parse)
{
  {
    MachO fmt("");
    EXPECT_FALSE(fmt.parse());
  }

  {
    // Something less than 4 bytes.
    auto file = tempFile("h");

    MachO fmt(file->fileName());
    EXPECT_FALSE(fmt.parse());
  }

  {
    // Something more than 4 bytes.
    auto file = tempFile("hello there, man");

    MachO fmt(file->fileName());
    EXPECT_FALSE(fmt.parse());
  }

  {
    MachO fmt(":macho_main");
    EXPECT_TRUE(fmt.parse());
    auto objs = fmt.objects();
    ASSERT_EQ(objs.size(), 1);
    EXPECT_EQ(objs[0]->cpuType(), CpuType::X86_64);
    EXPECT_EQ(objs[0]->fileType(), FileType::EXECUTE);
    const auto secs = objs[0]->sections();
    ASSERT_EQ(5, secs.size());
    EXPECT_EQ(Section::Type::TEXT, secs[0]->type());
    EXPECT_EQ(Section::Type::STRING, secs[1]->type());
    EXPECT_EQ(Section::Type::LC_VERSION_MIN_MACOSX, secs[2]->type());
    EXPECT_EQ(Section::Type::FUNC_STARTS, secs[3]->type());
    EXPECT_EQ(Section::Type::SYMBOLS, secs[4]->type());
  }

  {
    MachO fmt(":macho_main.o");
    EXPECT_TRUE(fmt.parse());
    auto objs = fmt.objects();
    ASSERT_EQ(objs.size(), 1);
    EXPECT_EQ(objs[0]->cpuType(), CpuType::X86_64);
    EXPECT_EQ(objs[0]->fileType(), FileType::OBJECT);
    const auto secs = objs[0]->sections();
    ASSERT_EQ(4, secs.size());
    EXPECT_EQ(Section::Type::TEXT, secs[0]->type());
    EXPECT_EQ(Section::Type::LC_VERSION_MIN_MACOSX, secs[1]->type());
    EXPECT_EQ(Section::Type::STRING, secs[2]->type());
    EXPECT_EQ(Section::Type::SYMBOLS, secs[3]->type());
  }

  {
    MachO fmt(":macho_main_32");
    EXPECT_TRUE(fmt.parse());
    auto objs = fmt.objects();
    ASSERT_EQ(objs.size(), 1);
    EXPECT_EQ(objs[0]->cpuType(), CpuType::X86);
    EXPECT_EQ(objs[0]->fileType(), FileType::EXECUTE);
    const auto secs = objs[0]->sections();
    ASSERT_EQ(5, secs.size());
    EXPECT_EQ(Section::Type::TEXT, secs[0]->type());
    EXPECT_EQ(Section::Type::STRING, secs[1]->type());
    EXPECT_EQ(Section::Type::LC_VERSION_MIN_MACOSX, secs[2]->type());
    EXPECT_EQ(Section::Type::FUNC_STARTS, secs[3]->type());
    EXPECT_EQ(Section::Type::SYMBOLS, secs[4]->type());
  }

  {
    MachO fmt(":macho_main_32.o");
    EXPECT_TRUE(fmt.parse());
    auto objs = fmt.objects();
    ASSERT_EQ(objs.size(), 1);
    EXPECT_EQ(objs[0]->cpuType(), CpuType::X86);
    EXPECT_EQ(objs[0]->fileType(), FileType::OBJECT);
    const auto secs = objs[0]->sections();
    ASSERT_EQ(4, secs.size());
    EXPECT_EQ(Section::Type::TEXT, secs[0]->type());
    EXPECT_EQ(Section::Type::LC_VERSION_MIN_MACOSX, secs[1]->type());
    EXPECT_EQ(Section::Type::STRING, secs[2]->type());
    EXPECT_EQ(Section::Type::SYMBOLS, secs[3]->type());
  }

  {
    MachO fmt(":macho_main_32_64");
    EXPECT_TRUE(fmt.parse());
    auto objs = fmt.objects();
    ASSERT_EQ(objs.size(), 2);
    EXPECT_EQ(objs[0]->cpuType(), CpuType::X86);
    EXPECT_EQ(objs[0]->fileType(), FileType::EXECUTE);
    auto secs = objs[0]->sections();
    ASSERT_EQ(5, secs.size());
    EXPECT_EQ(Section::Type::TEXT, secs[0]->type());
    EXPECT_EQ(Section::Type::STRING, secs[1]->type());
    EXPECT_EQ(Section::Type::LC_VERSION_MIN_MACOSX, secs[2]->type());
    EXPECT_EQ(Section::Type::FUNC_STARTS, secs[3]->type());
    EXPECT_EQ(Section::Type::SYMBOLS, secs[4]->type());

    EXPECT_EQ(objs[1]->cpuType(), CpuType::X86_64);
    EXPECT_EQ(objs[1]->fileType(), FileType::EXECUTE);
    secs = objs[1]->sections();
    ASSERT_EQ(5, secs.size());
    EXPECT_EQ(Section::Type::TEXT, secs[0]->type());
    EXPECT_EQ(Section::Type::STRING, secs[1]->type());
    EXPECT_EQ(Section::Type::LC_VERSION_MIN_MACOSX, secs[2]->type());
    EXPECT_EQ(Section::Type::FUNC_STARTS, secs[3]->type());
    EXPECT_EQ(Section::Type::SYMBOLS, secs[4]->type());
  }

  {
    MachO fmt(":macho_func");
    EXPECT_TRUE(fmt.parse());
    auto objs = fmt.objects();
    ASSERT_EQ(objs.size(), 1);
    EXPECT_EQ(objs[0]->cpuType(), CpuType::X86_64);
    EXPECT_EQ(objs[0]->fileType(), FileType::EXECUTE);
    const auto secs = objs[0]->sections();
    ASSERT_EQ(5, secs.size());
    EXPECT_EQ(Section::Type::TEXT, secs[0]->type());
    EXPECT_EQ(Section::Type::STRING, secs[1]->type());
    EXPECT_EQ(Section::Type::LC_VERSION_MIN_MACOSX, secs[2]->type());
    EXPECT_EQ(Section::Type::FUNC_STARTS, secs[3]->type());
    EXPECT_EQ(Section::Type::SYMBOLS, secs[4]->type());
  }

  {
    MachO fmt(":macho_strings");
    EXPECT_TRUE(fmt.parse());

    auto objs = fmt.objects();
    ASSERT_EQ(objs.size(), 1);
    EXPECT_EQ(objs[0]->cpuType(), CpuType::X86_64);
    EXPECT_EQ(objs[0]->fileType(), FileType::EXECUTE);

    auto sec = objs[0]->section(Section::Type::CSTRING);
    ASSERT_NE(sec, nullptr);

    auto strings = CStringReader(sec->data()).readAll();
    ASSERT_EQ(strings.size(), 2);
    EXPECT_EQ(strings[0], "hello");
    EXPECT_EQ(strings[1], "second");

    const auto secs = objs[0]->sections();
    ASSERT_EQ(8, secs.size());
    EXPECT_EQ(Section::Type::TEXT, secs[0]->type());
    EXPECT_EQ(Section::Type::SYMBOL_STUBS, secs[1]->type());
    EXPECT_EQ(Section::Type::CSTRING, secs[2]->type());
    EXPECT_EQ(Section::Type::STRING, secs[3]->type());
    EXPECT_EQ(Section::Type::LC_VERSION_MIN_MACOSX, secs[4]->type());
    EXPECT_EQ(Section::Type::FUNC_STARTS, secs[5]->type());
    EXPECT_EQ(Section::Type::SYMBOLS, secs[6]->type());
    EXPECT_EQ(Section::Type::DYN_SYMBOLS, secs[7]->type());
  }

  {
    MachO fmt(":macho_main_objc");
    EXPECT_TRUE(fmt.parse());
    auto objs = fmt.objects();
    ASSERT_EQ(objs.size(), 1);
    EXPECT_EQ(objs[0]->cpuType(), CpuType::X86_64);
    EXPECT_EQ(objs[0]->fileType(), FileType::EXECUTE);
    const auto secs = objs[0]->sections();
    ASSERT_EQ(9, secs.size());
    EXPECT_EQ(Section::Type::TEXT, secs[0]->type());
    EXPECT_EQ(Section::Type::SYMBOL_STUBS, secs[1]->type());
    EXPECT_EQ(Section::Type::CSTRING, secs[2]->type());
    EXPECT_EQ(Section::Type::CSTRING, secs[3]->type());
    EXPECT_EQ(Section::Type::STRING, secs[4]->type());
    EXPECT_EQ(Section::Type::LC_VERSION_MIN_MACOSX, secs[5]->type());
    EXPECT_EQ(Section::Type::FUNC_STARTS, secs[6]->type());
    EXPECT_EQ(Section::Type::SYMBOLS, secs[7]->type());
    EXPECT_EQ(Section::Type::DYN_SYMBOLS, secs[8]->type());
  }

  {
    MachO fmt(":macho_lib.dylib");
    EXPECT_TRUE(fmt.parse());
    auto objs = fmt.objects();
    ASSERT_EQ(objs.size(), 1);
    EXPECT_EQ(objs[0]->cpuType(), CpuType::X86_64);
    EXPECT_EQ(objs[0]->fileType(), FileType::DYLIB);
    const auto secs = objs[0]->sections();
    ASSERT_EQ(5, secs.size());
    EXPECT_EQ(Section::Type::TEXT, secs[0]->type());
    EXPECT_EQ(Section::Type::STRING, secs[1]->type());
    EXPECT_EQ(Section::Type::LC_VERSION_MIN_MACOSX, secs[2]->type());
    EXPECT_EQ(Section::Type::FUNC_STARTS, secs[3]->type());
    EXPECT_EQ(Section::Type::SYMBOLS, secs[4]->type());
  }
}
