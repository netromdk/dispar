#include "gtest/gtest.h"

#include "testutils.h"

#include "Project.h"

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
    auto spy = SIGNAL_SPY_ZERO(&p, &Project::tagsChanged);
    auto spy2 = SIGNAL_SPY_ZERO(&p, &Project::modified);
    EXPECT_TRUE(p.addAddressTag(tag, addr));
  }

  {
    auto spy = SIGNAL_SPY_ZERO(&p, &Project::tagsChanged);
    spy->setExpect(false);

    auto spy2 = SIGNAL_SPY_ZERO(&p, &Project::modified);
    spy2->setExpect(false);

    EXPECT_FALSE(p.addAddressTag(tag, addr)); // Ignore existent.
  }

  auto tags = p.addressTags(addr);
  ASSERT_EQ(tags.size(), 1);
  EXPECT_EQ(tags[0], tag);

  {
    auto spy = SIGNAL_SPY_ZERO(&p, &Project::tagsChanged);
    spy->setExpect(false);

    auto spy2 = SIGNAL_SPY_ZERO(&p, &Project::modified);
    spy2->setExpect(false);

    // Can't add same tag to different address.
    quint64 addr2 = 0x4321;
    EXPECT_FALSE(p.addAddressTag(tag, addr2));
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
    auto spy = SIGNAL_SPY_ZERO(&p, &Project::tagsChanged);
    auto spy2 = SIGNAL_SPY_ZERO(&p, &Project::modified);
    EXPECT_TRUE(p.removeAddressTag(tag, addr));
  }

  {
    auto spy = SIGNAL_SPY_ZERO(&p, &Project::tagsChanged);
    spy->setExpect(false);

    auto spy2 = SIGNAL_SPY_ZERO(&p, &Project::modified);
    spy2->setExpect(false);

    // Not removed because already removed.
    EXPECT_FALSE(p.removeAddressTag(tag, addr));
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

  QList<QPair<QString, quint64>> tags{qMakePair(tag, addr), qMakePair(tag2, addr2)};
  {
    auto spy = SIGNAL_SPY_ZERO(&p, &Project::tagsChanged);
    auto spy2 = SIGNAL_SPY_ZERO(&p, &Project::modified);
    EXPECT_TRUE(p.removeAddressTags(tags));
  }

  {
    auto spy = SIGNAL_SPY_ZERO(&p, &Project::tagsChanged);
    spy->setExpect(false);

    auto spy2 = SIGNAL_SPY_ZERO(&p, &Project::modified);
    spy2->setExpect(false);

    // Not removed because already removed.
    EXPECT_FALSE(p.removeAddressTags(tags));
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
