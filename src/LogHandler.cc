#include "LogHandler.h"
#include "Constants.h"
#include "Context.h"
#include "Util.h"
#include "cxx.h"

#include <cassert>
#include <iostream>

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
  case QtInfoMsg:
    return tr("Info");
  case QtWarningMsg:
    return tr("Warning");
  case QtCriticalMsg:
    return tr("Critical");
  case QtFatalMsg:
    return tr("Fatal");
  }

  // This won't be reached.
  return {};
}

LogHandler::LogHandler(Context &context) : entries_(Constants::Log::MEMORY_ENTRIES)
{
  ASSERT_X(!instance, "Only one LogHandler can be live at any one time");
  instance = this;

  ctx = &context;

  openLogFile();

  connect(&fileFlushTimer, &QTimer::timeout, this, &LogHandler::flushToFile);
  fileFlushTimer.setInterval(Constants::Log::FLUSH_INTERVAL);
  fileFlushTimer.start();

  qInstallMessageHandler(dispar::LogHandler::messageHandler);
}

LogHandler::~LogHandler()
{
  assert(instance);
  instance = nullptr;

  // Ensure last chunk is written to disk.
  flushToFile();
}

void LogHandler::registerType()
{
  qRegisterMetaType<Entry>("Entry");
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
  }

  // This won't be reached.
  return {};
}

QString LogHandler::messageString(QtMsgType type, const QMessageLogContext &context,
                                  const QString &msg)
{
  auto output = msg;

  // Only show function/file/line in debug.
#ifndef NDEBUG
  // Include file name and two immediate parent folder names.
  QString file;
  if (context.file != nullptr) {
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

  return output;
}

void LogHandler::messageHandler(QtMsgType type, const QMessageLogContext &context,
                                const QString &msg)
{
  static QMutex mutex;

  // In debug mode, assert on important log messages, like QObject signal connections.
  // Examples:
  //   QObject::connect: Cannot queue arguments of type 'Entry'
  //   QObject: Cannot create children for a parent that is in a different thread.
#ifndef NDEBUG
  if (const auto lmsg = msg.trimmed().toLower();
      lmsg.startsWith("qobject::connect:") || lmsg.startsWith("qobject::disconnect:") ||
      lmsg.contains("qobject: cannot create children for a parent that is in a different thread")) {
    std::cerr << msg.toStdString() << std::endl;
    assert(false);
    return;
  }
#endif

  // Ignore log message if level is lower than current log level, except if verbose is enabled.
  if (!ctx->acceptMsgType(type)) {
    return;
  }

  const auto output = messageString(type, context, msg);

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

  if (instance != nullptr) {
    instance->addEntry({QDateTime::currentDateTime(), type, output});
  }
}

void LogHandler::addEntry(const Entry &entry)
{
  entries_.append(entry);
  queueToFile(entry);
  emit newEntry(entry);
}

const LogHandler::Container &LogHandler::entries() const
{
  return entries_;
}

QString LogHandler::logPath()
{
  return QDir::home().absoluteFilePath(".dispar.log");
}

void LogHandler::flushToFile()
{
  QMutexLocker locker(&chunkMutex);
  if (logFile.isOpen() && !nextChunk.isEmpty()) {
    logFile.write(nextChunk);
    logFile.flush();
    nextChunk.clear();
  }
}

void LogHandler::queueToFile(const Entry &entry)
{
  QString output("[");
  output.append(QString("%1 - ").arg(entry.time.toString(Qt::ISODate)));

  switch (entry.type) {
  case QtDebugMsg:
    output.append("DD");
    break;

  case QtInfoMsg:
    output.append("II");
    break;

  case QtWarningMsg:
    output.append("WW");
    break;

  case QtCriticalMsg:
    output.append("CC");
    break;

  case QtFatalMsg:
    output.append("FF");
    break;
  }
  output.append("] ");

  output += entry.msg;
  if (!output.endsWith("\n")) {
    output.append("\n");
  }

  {
    QMutexLocker locker(&chunkMutex);
    nextChunk.append(output.toUtf8());
  }

  // Flush important messages to disk immediately.
  if (msgLogLevel(entry.type) >= Constants::Log::WARNING_LEVEL) {
    flushToFile();
  }
}

void LogHandler::openLogFile()
{
  logFile.setFileName(logPath());

  auto flags = QIODevice::Text | QIODevice::Append;
  if (logFile.exists() && logFile.size() > Constants::Log::MAX_SIZE) {
    flags |= QIODevice::Truncate;
    std::cout << "Truncating log file due to size >"
              << Util::formatSize(Constants::Log::MAX_SIZE).toStdString() << ": "
              << Util::formatSize(logFile.size()).toStdString() << std::endl;
  }

  if (!logFile.open(flags)) {
    std::cerr << "Could not open log file for writing: " << logFile.fileName().toStdString()
              << " error=" << logFile.error() << std::endl;
  }
}

} // namespace dispar
