#include "MacSdkVersionPatcher.h"
#include "Reader.h"
#include "Section.h"
#include "Util.h"

#include <QBuffer>

namespace dispar {

MacSdkVersionPatcher::MacSdkVersionPatcher(Section &section_) : section(section_)
{
  parse();
}

bool MacSdkVersionPatcher::valid() const
{
  return valid_;
}

MacSdkVersionPatcher::Version MacSdkVersionPatcher::target() const
{
  return target_;
}

bool MacSdkVersionPatcher::setTarget(const Version &version)
{
  if (version == target_) {
    return false;
  }

  target_ = version;
  patch(version, 0);
  return true;
}

MacSdkVersionPatcher::Version MacSdkVersionPatcher::sdk() const
{
  return sdk_;
}

bool MacSdkVersionPatcher::setSdk(const Version &version)
{
  if (version == sdk_) {
    return false;
  }

  sdk_ = version;
  patch(version, 4);
  return true;
}

void MacSdkVersionPatcher::parse()
{
  valid_ = false;

  if (section.data().isEmpty() || section.data().size() != 4 * 2 ||
      (section.type() != Section::Type::LC_VERSION_MIN_MACOSX &&
       section.type() != Section::Type::LC_VERSION_MIN_IPHONEOS &&
       section.type() != Section::Type::LC_VERSION_MIN_WATCHOS &&
       section.type() != Section::Type::LC_VERSION_MIN_TVOS)) {
    return;
  }

  QBuffer buf;
  buf.setData(section.data());
  buf.open(QIODevice::ReadOnly);

  Reader reader(buf);

  bool ok = false;
  const auto targetVersion = reader.getUInt32(&ok);
  if (!ok) return;
  target_ = Util::decodeMacSdkVersion(targetVersion);

  const auto sdkVersion = reader.getUInt32(&ok);
  if (!ok) return;
  sdk_ = Util::decodeMacSdkVersion(sdkVersion);

  valid_ = true;
}

void MacSdkVersionPatcher::patch(const Version &newVersion, int pos)
{
  QByteArray data;
  data.append(Util::longToData(Util::encodeMacSdkVersion(newVersion)));

  // Set as "sub" data so that the modified flag is set, too.
  section.setSubData(data, pos);
}

} // namespace dispar
