#include "gtest/gtest.h"

#include "testutils.h"

#include <memory>

#include "BinaryObject.h"
#include "MacSdkVersionPatcher.h"
#include "Section.h"
#include "Util.h"
#include "formats/MachO.h"
using namespace dispar;

using Version = MacSdkVersionPatcher::Version;

QByteArray createData(const Version &target, const Version &sdk)
{
  QByteArray data;
  data.append(Util::longToData(Util::encodeMacSdkVersion(target)));
  data.append(Util::longToData(Util::encodeMacSdkVersion(sdk)));
  return data;
}

std::unique_ptr<Section>
createSection(const QByteArray &data,
              const Section::Type type = Section::Type::LC_VERSION_MIN_MACOSX)
{
  const quint64 addr(0x1000);
  const quint64 size = data.size();
  auto section = std::make_unique<Section>(type, "Test", addr, size);
  section->setData(data);
  return section;
}

TEST(MacSdkVersionPatcher, noData)
{
  QByteArray data;
  auto section = createSection(data);

  MacSdkVersionPatcher patcher(*section.get());
  EXPECT_FALSE(patcher.valid());
}

TEST(MacSdkVersionPatcher, notLcVersionMinMacosx)
{
  QByteArray data;
  data.append("hello");

  const quint64 addr(0x1000);
  const quint64 size = data.size();
  Section section(Section::Type::TEXT, "Test", addr, size);
  section.setData(data);

  MacSdkVersionPatcher patcher(section);
  EXPECT_FALSE(patcher.valid());
}

TEST(MacSdkVersionPatcher, invalidData)
{
  for (const auto &data :
       {"a", "ab", "abc", "abcd", "abcde", "abcdef", "abcdef0", "abcdef012", "abcdef0123"}) {
    auto section = createSection(data);

    MacSdkVersionPatcher patcher(*section.get());
    EXPECT_FALSE(patcher.valid()) << data;
  }
}

TEST(MacSdkVersionPatcher, macOS)
{
  const auto data = createData({10, 12}, {10, 15});
  auto section = createSection(data, dispar::Section::Type::LC_VERSION_MIN_MACOSX);

  MacSdkVersionPatcher patcher(*section.get());
  ASSERT_TRUE(patcher.valid());
}

TEST(MacSdkVersionPatcher, iOS)
{
  const auto data = createData({10, 12}, {10, 15});
  auto section = createSection(data, dispar::Section::Type::LC_VERSION_MIN_IPHONEOS);

  MacSdkVersionPatcher patcher(*section.get());
  ASSERT_TRUE(patcher.valid());
}

TEST(MacSdkVersionPatcher, watchOS)
{
  const auto data = createData({10, 12}, {10, 15});
  auto section = createSection(data, dispar::Section::Type::LC_VERSION_MIN_WATCHOS);

  MacSdkVersionPatcher patcher(*section.get());
  ASSERT_TRUE(patcher.valid());
}

TEST(MacSdkVersionPatcher, tvOS)
{
  const auto data = createData({10, 12}, {10, 15});
  auto section = createSection(data, dispar::Section::Type::LC_VERSION_MIN_TVOS);

  MacSdkVersionPatcher patcher(*section.get());
  ASSERT_TRUE(patcher.valid());
}

TEST(MacSdkVersionPatcher, read)
{
  const auto data = createData({10, 12}, {10, 15});
  auto section = createSection(data);

  MacSdkVersionPatcher patcher(*section.get());
  ASSERT_TRUE(patcher.valid());

  EXPECT_EQ((Version{10, 12}), patcher.target());
  EXPECT_EQ((Version{10, 15}), patcher.sdk());
}

TEST(MacSdkVersionPatcher, sameVersions)
{
  const auto data = createData({10, 12}, {10, 15});
  auto section = createSection(data);

  MacSdkVersionPatcher patcher(*section.get());
  ASSERT_TRUE(patcher.valid());

  EXPECT_FALSE(patcher.setTarget(patcher.target()));
  EXPECT_FALSE(patcher.setSdk(patcher.sdk()));
}

TEST(MacSdkVersionPatcher, changeVersions)
{
  const auto data = createData({10, 12}, {10, 15});
  auto section = createSection(data);
  EXPECT_FALSE(section->isModified());

  MacSdkVersionPatcher patcher(*section.get());
  ASSERT_TRUE(patcher.valid());

  const Version target{10, 11};
  ASSERT_TRUE(patcher.setTarget(target));

  QList<QPair<int, int>> mod{{0, 4}};
  EXPECT_EQ(mod, section->modifiedRegions());

  const Version sdk{10, 14};
  ASSERT_TRUE(patcher.setSdk(sdk));

  mod = decltype(mod){{0, 4}, {4, 4}};
  EXPECT_EQ(mod, section->modifiedRegions());

  EXPECT_EQ(target, patcher.target());
  EXPECT_EQ(sdk, patcher.sdk());

  EXPECT_EQ(createData(target, sdk), section->data());
  EXPECT_TRUE(section->isModified());
}

TEST(MacSdkVersionPatcher, universalBinary)
{
  MachO fmt(":macho_main_32_64");
  ASSERT_TRUE(fmt.parse());

  const auto objects = fmt.objects();
  ASSERT_EQ(2, objects.size());

  for (auto object : objects) {
    auto *section = object->section(Section::Type::LC_VERSION_MIN_MACOSX);
    ASSERT_NE(nullptr, section);

    MacSdkVersionPatcher patcher(*section);
    ASSERT_TRUE(patcher.valid());

    const MacSdkVersionPatcher::Version target{10, 11}, sdk{10, 11};
    EXPECT_EQ(target, patcher.target());
    EXPECT_EQ(sdk, patcher.sdk());
  }
}
