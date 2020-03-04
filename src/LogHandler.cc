#include "LogHandler.h"
#include "Context.h"
#include "cxx.h"

#include <cassert>

#include <QDir>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QTextStream>

namespace dispar {

namespace {

// Make context and handler available in static messageHandler().
Context *ctx = nullptr;
LogHandler *instance = nullptr;

} // namespace

QString LogHandler::Entry::typeString() const
{
  switch (type) {
  case QtDebugMsg:
    return tr("Debug");
  default:
  case QtInfoMsg:
    return tr("Info");
  case QtWarningMsg:
    return tr("Warning");
  case QtCriticalMsg:
    return tr("Critical");
  case QtFatalMsg:
    return tr("Fatal");
  }
}

LogHandler::LogHandler(Context &context)
{
  ASSERT_X(!instance, "Only one LogHandler can be live at any one time");
  instance = this;

  ctx = &context;

  qInstallMessageHandler(this->messageHandler);
}

LogHandler::~LogHandler()
{
  assert(instance);
  instance = nullptr;
}

int LogHandler::msgLogLevel(QtMsgType type)
{
  switch (type) {
  case QtDebugMsg:
    return Constants::Log::DEBUG_LEVEL;
  case QtInfoMsg:
    return Constants::Log::INFO_LEVEL;
  case QtWarningMsg:
    return Constants::Log::WARNING_LEVEL;
  case QtCriticalMsg:
    return Constants::Log::CRITICAL_LEVEL;
  case QtFatalMsg:
    return Constants::Log::FATAL_LEVEL;
  default:
    return Constants::Log::DEFAULT_LEVEL;
  }
}

void LogHandler::messageHandler(QtMsgType type, const QMessageLogContext &context,
                                const QString &msg)
{
  static QMutex mutex;

  // Ignore log message if level is lower than current log level, except if verbose is enabled.
  if (!ctx->acceptMsgType(type)) {
    return;
  }

  auto output = msg;

  // Only show function/file/line in debug.
#ifndef NDEBUG
  // Include file name and two immediate parent folder names.
  QString file;
  if (context.file) {
    const QFileInfo fi(context.file);
    const auto path = fi.absoluteFilePath();
    auto parent = QFileInfo(path).dir();
    QStringList parts{parent.dirName()};
    if (parent.cdUp()) {
      parts.prepend(parent.dirName());
    }
    parts << fi.fileName();
    file = parts.join("/") + ":";
  }

  // const char *function = context.function ? context.function : "";
  // output += QString(" {%1 @ %2:%3}").arg(function).arg(file).arg(context.line);
  output += QString(" {%1%2}").arg(file).arg(context.line);
#endif

  // Lock mutex while writing to stdout such that output isn't messed up when coming from multiple
  // threads.
  {
    QMutexLocker locker(&mutex);

    QTextStream stream(stdout);
    stream << output;
    if (!output.endsWith("\n")) {
      stream << "\n";
    }
  }

  if (instance) {
    instance->addEntry({QDateTime::currentDateTime(), type, msg});
  }
}

void LogHandler::addEntry(const Entry &entry)
{
  entries_ << entry;
  emit newEntry(entry);
}

QList<LogHandler::Entry> LogHandler::entries() const
{
  return entries_;
}

} // namespace dispar
