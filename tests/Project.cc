#include <QSignalSpy>

#include "gtest/gtest.h"

#include "testutils.h"

#include "Project.h"
using namespace dispar;

TEST(Project, instantiate)
{
  Project p;
}

TEST(Project, save)
{
  Project p;
  p.setBinary("something.o");

  auto file = tempFile();
  auto path = file->fileName();
  ASSERT_TRUE(p.save(path)) << "Could not save to: " << path;
  EXPECT_EQ(p.file(), path) << p.file();
}

TEST(Project, saveHasFileAlready)
{
  Project p;
  p.setBinary("something.o");

  auto file = tempFile();
  auto path = file->fileName();
  ASSERT_TRUE(p.save(path)) << "Could not save to: " << path;
  EXPECT_EQ(p.file(), path) << p.file();

  p.setBinary("another.dylib");
  ASSERT_TRUE(p.save());
  EXPECT_EQ(p.file(), path) << p.file();
}

TEST(Project, load)
{
  Project p;
  p.setBinary("something.o");

  auto file = tempFile();
  auto path = file->fileName();
  ASSERT_TRUE(p.save(path)) << "Could not save to: " << path;

  auto p2 = Project::load(path);
  ASSERT_NE(p2, nullptr) << "Could not load from: " << path;
  EXPECT_EQ(p2->binary(), p.binary()) << p2->binary() << p.binary();
  EXPECT_EQ(p2->file(), path) << p2->file();
}

TEST(Project, loadFromAlreadyKnownFile)
{
  Project p;
  p.setBinary("something.o");

  auto file = tempFile();
  auto path = file->fileName();
  ASSERT_TRUE(p.save(path)) << "Could not save to: " << path;

  // Make sure the values are correctly saved when giving no arguments.
  ASSERT_TRUE(p.save());

  auto p2 = Project::load(path);
  ASSERT_NE(p2, nullptr) << "Could not load from: " << path;
  EXPECT_EQ(p2->binary(), p.binary()) << p2->binary() << p.binary();
}

TEST(Project, addTag)
{
  Project p;

  quint64 addr = 0x1234;
  QString tag("tag");

  {
    QSignalSpy tagsChangedSpy(&p, &Project::tagsChanged);
    QSignalSpy modifiedSpy(&p, &Project::modified);

    EXPECT_TRUE(p.addAddressTag(tag, addr));

    EXPECT_EQ(1, tagsChangedSpy.count());
    EXPECT_EQ(1, modifiedSpy.count());
  }

  {
    QSignalSpy tagsChangedSpy(&p, &Project::tagsChanged);
    QSignalSpy modifiedSpy(&p, &Project::modified);

    EXPECT_FALSE(p.addAddressTag(tag, addr)); // Ignore existent.

    EXPECT_EQ(0, tagsChangedSpy.count());
    EXPECT_EQ(0, modifiedSpy.count());
  }

  auto tags = p.addressTags(addr);
  ASSERT_EQ(tags.size(), 1);
  EXPECT_EQ(tags[0], tag);

  {
    QSignalSpy tagsChangedSpy(&p, &Project::tagsChanged);
    QSignalSpy modifiedSpy(&p, &Project::modified);

    // Can't add same tag to different address.
    quint64 addr2 = 0x4321;
    EXPECT_FALSE(p.addAddressTag(tag, addr2));

    EXPECT_EQ(0, tagsChangedSpy.count());
    EXPECT_EQ(0, modifiedSpy.count());
  }
}

TEST(Project, removeTag)
{
  Project p;

  quint64 addr = 0x1234;
  QString tag("tag");
  EXPECT_TRUE(p.addAddressTag(tag, addr));
  EXPECT_FALSE(p.addAddressTag(tag, addr)); // Ignore existent.

  auto tags = p.addressTags(addr);
  ASSERT_EQ(tags.size(), 1);
  EXPECT_EQ(tags[0], tag);

  {
    QSignalSpy tagsChangedSpy(&p, &Project::tagsChanged);
    QSignalSpy modifiedSpy(&p, &Project::modified);

    EXPECT_TRUE(p.removeAddressTag(tag));

    EXPECT_EQ(1, tagsChangedSpy.count());
    EXPECT_EQ(1, modifiedSpy.count());
  }

  {
    QSignalSpy tagsChangedSpy(&p, &Project::tagsChanged);
    QSignalSpy modifiedSpy(&p, &Project::modified);

    // Not removed because already removed.
    EXPECT_FALSE(p.removeAddressTag(tag));

    EXPECT_EQ(0, tagsChangedSpy.count());
    EXPECT_EQ(0, modifiedSpy.count());
  }

  EXPECT_EQ(p.addressTags(addr).size(), 0);
}

TEST(Project, removeTags)
{
  Project p;

  quint64 addr = 0x1234;
  QString tag("tag");
  EXPECT_TRUE(p.addAddressTag(tag, addr));
  ASSERT_EQ(p.addressTags(addr).size(), 1);

  quint64 addr2 = 0x4321;
  QString tag2("tag2");
  EXPECT_TRUE(p.addAddressTag(tag2, addr2));
  ASSERT_EQ(p.addressTags(addr2).size(), 1);

  QStringList tags{tag, tag2};
  {
    QSignalSpy tagsChangedSpy(&p, &Project::tagsChanged);
    QSignalSpy modifiedSpy(&p, &Project::modified);

    EXPECT_TRUE(p.removeAddressTags(tags));

    EXPECT_EQ(1, tagsChangedSpy.count());
    EXPECT_EQ(1, modifiedSpy.count());
  }

  {
    QSignalSpy tagsChangedSpy(&p, &Project::tagsChanged);
    QSignalSpy modifiedSpy(&p, &Project::modified);

    // Not removed because already removed.
    EXPECT_FALSE(p.removeAddressTags(tags));

    EXPECT_EQ(0, tagsChangedSpy.count());
    EXPECT_EQ(0, modifiedSpy.count());
  }

  EXPECT_EQ(p.addressTags(addr).size(), 0);
  EXPECT_EQ(p.addressTags(addr2).size(), 0);
}

TEST(Project, saveLoadTags)
{
  Project p;

  quint64 addr = 0x1234;
  QString tag("tag");
  EXPECT_TRUE(p.addAddressTag(tag, addr));
  EXPECT_FALSE(p.addAddressTag(tag, addr)); // Ignore existent.

  auto tags = p.addressTags(addr);
  ASSERT_EQ(tags.size(), 1);
  EXPECT_EQ(tags[0], tag);

  auto file = tempFile();
  auto path = file->fileName();
  ASSERT_TRUE(p.save(path)) << "Could not save to: " << path;

  auto p2 = Project::load(path);
  ASSERT_NE(p2, nullptr) << "Could not load from: " << path;

  auto tags2 = p2->addressTags(addr);
  EXPECT_EQ(tags, tags2) << "Tags were not saved/loaded correctly";
}

TEST(Project, saveLoadModifiesRegions)
{
  Project p;

  quint64 addr = 0x1234;
  QByteArray data("1234");
  p.addModifiedRegion(addr, data);

  quint64 addr2 = 0x10304;
  QByteArray data2("Hello, World!");
  p.addModifiedRegion(addr2, data2);

  auto &modRegs = p.modifiedRegions();
  ASSERT_EQ(modRegs.size(), 2);
  ASSERT_TRUE(modRegs.contains(addr));
  EXPECT_EQ(modRegs[addr], data);
  ASSERT_TRUE(modRegs.contains(addr2));
  EXPECT_EQ(modRegs[addr2], data2);

  auto file = tempFile();
  auto path = file->fileName();
  ASSERT_TRUE(p.save(path)) << "Could not save to: " << path;

  auto p2 = Project::load(path);
  ASSERT_NE(p2, nullptr) << "Could not load from: " << path;

  auto modRegs2 = p2->modifiedRegions();
  EXPECT_EQ(modRegs, modRegs2) << "Modified regions were not saved/loaded correctly";

  p2->clearModifiedRegions();
  EXPECT_EQ(p2->modifiedRegions().size(), 0);
}
