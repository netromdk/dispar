#ifndef SRC_LOGHANDLER_H
#define SRC_LOGHANDLER_H

#include <QContiguousCache>
#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QMutex>
#include <QObject>
#include <QTimer>

namespace dispar {

class Context;

class LogHandler : public QObject {
  Q_OBJECT

public:
  struct Entry {
    QDateTime time;
    QtMsgType type = QtMsgType::QtInfoMsg;
    QString msg;
    [[nodiscard]] QString typeString() const;
  };

  using Container = QContiguousCache<Entry>;

  LogHandler(Context &context);
  ~LogHandler() override;

  static void registerType();

  /// Log level corresponding to \p type.
  static int msgLogLevel(QtMsgType type);

  /// Create log message string from \p type, \p context, and \p msg.
  static QString messageString(QtMsgType type, const QMessageLogContext &context,
                               const QString &msg);

  /// Message handler to be installed via \p qInstallMessageHandler().
  static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

  void addEntry(const Entry &entry);
  const Container &entries() const;

  static QString logPath();

signals:
  void newEntry(const Entry &entry);

private slots:
  void flushToFile();

private:
  void queueToFile(const Entry &entry);
  void openLogFile();

  Container entries_;
  QFile logFile;
  QByteArray nextChunk;
  QTimer fileFlushTimer;
  QMutex chunkMutex;
};

} // namespace dispar

#endif // SRC_LOGHANDLER_H
