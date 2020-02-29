#ifndef SRC_LOGHANDLER_H
#define SRC_LOGHANDLER_H

#include <QDebug>

namespace dispar {

class Context;

class LogHandler {
public:
  LogHandler(Context &context);

  /// Message handler to be installed via \p qInstallMessageHandler().
  static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
};

} // namespace dispar

#endif // SRC_LOGHANDLER_H
