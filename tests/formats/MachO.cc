#include "gtest/gtest.h"

#include "testutils.h"

#include "formats/MachO.h"

TEST(MachO, instantiate)
{
  MachO fmt("some_file");
  EXPECT_EQ(fmt.type(), Format::Type::MachO);
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
    EXPECT_EQ(fmt.objects().size(), 1);
  }

  {
    MachO fmt(":macho_main.o");
    EXPECT_TRUE(fmt.parse());
    EXPECT_EQ(fmt.objects().size(), 1);
  }

  {
    MachO fmt(":macho_main_32");
    EXPECT_TRUE(fmt.parse());
    EXPECT_EQ(fmt.objects().size(), 1);
  }

  {
    MachO fmt(":macho_main_32.o");
    EXPECT_TRUE(fmt.parse());
    EXPECT_EQ(fmt.objects().size(), 1);
  }

  {
    MachO fmt(":macho_main_32_64");
    EXPECT_TRUE(fmt.parse());
    EXPECT_EQ(fmt.objects().size(), 2);
  }
}
