#ifndef DISPAR_VERSION_H
#define DISPAR_VERSION_H

#include <QString>
#include <QVersionNumber>

namespace dispar {

static constexpr int MAJOR_VERSION = 0;
static constexpr int MINOR_VERSION = 2;
static const QString VERSION_DATE("November 23, 2017");

static inline QString versionString(int major, int minor, bool showDate = false)
{
  return QVersionNumber({major, minor}).toString() +
         (showDate ? " [" + QString(VERSION_DATE) + "]" : "");
}

static inline QString versionString(bool showDate = false)
{
  return versionString(MAJOR_VERSION, MINOR_VERSION, showDate);
}

} // namespace dispar

#endif // DISPAR_VERSION_H
