#include "LogHandler.h"
#include "Context.h"

#include <QDir>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QTextStream>

namespace dispar {

namespace {

// Make context available in static messageHandler().
Context *ctx = nullptr;

} // namespace

LogHandler::LogHandler(Context &context)
{
  ctx = &context;
  qInstallMessageHandler(this->messageHandler);
}

void LogHandler::messageHandler(QtMsgType type, const QMessageLogContext &context,
                                const QString &msg)
{
  static QMutex mutex;

  // Ignore debug messages in release mode, except if verbose is enabled.
#ifdef NDEBUG
  if (type == QtDebugMsg && !ctx->verbose()) {
    return;
  }
#endif

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
  QMutexLocker locker(&mutex);

  QTextStream stream(stdout);
  stream << output;
  if (!output.endsWith("\n")) {
    stream << "\n";
  }
}

} // namespace dispar
