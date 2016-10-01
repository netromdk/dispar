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
}
