#ifndef SRC_LOGHANDLER_H
#define SRC_LOGHANDLER_H

#include <QDebug>

namespace dispar {

class LogHandler {
public:
  LogHandler();

  /// Message handler to be installed via \p qInstallMessageHandler().
  static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);
};

} // namespace dispar

#endif // SRC_LOGHANDLER_H
