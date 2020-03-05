#ifndef SRC_CONSTANTS_H
#define SRC_CONSTANTS_H

#include <QString>

#include <capstone.h>

#include "LogHandler.h"

namespace dispar {
namespace Constants {

static const QString PROJECT_URL("https://github.com/netromdk/dispar");

namespace Log {

enum {
  DEBUG_LEVEL = 0,
  INFO_LEVEL = 1,
  WARNING_LEVEL = 2,
  CRITICAL_LEVEL = 3,
  FATAL_LEVEL = 4,

  DEFAULT_LEVEL = INFO_LEVEL,
};

/// Maximum number of log entries to keep in memory in a sliding window fashion.
static const int MEMORY_ENTRIES = 256;

} // namespace Log

namespace Debugger {

/// Timeout in milliseconds when testing if debugger is runnable.
static constexpr int runnableTimeout = 1000;

} // namespace Debugger

namespace Deps {

namespace Qt {

static const QString NAME("Qt");
static const QString URL("https://www.qt.io");
static const QString VERSION(QT_VERSION_STR);

} // namespace Qt

namespace Capstone {

static const QString NAME("Capstone");
static const QString URL("http://www.capstone-engine.org");
static const QString VERSION(QString("%1.%2").arg(CS_API_MAJOR).arg(CS_API_MINOR));

} // namespace Capstone

namespace Libiberty {

static const QString NAME("libiberty");
static const QString URL("https://www.gnu.org/software/binutils");
static const QString VERSION("2.27");

} // namespace Libiberty

} // namespace Deps

} // namespace Constants
} // namespace dispar

#endif // SRC_CONSTANTS_H
