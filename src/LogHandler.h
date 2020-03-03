#ifndef SRC_LOGHANDLER_H
#define SRC_LOGHANDLER_H

#include <QDateTime>
#include <QDebug>
#include <QObject>

namespace dispar {

class Context;

class LogHandler : public QObject {
  Q_OBJECT

public:
  struct Entry {
    QDateTime time;
    QtMsgType type = QtMsgType::QtInfoMsg;
    QString msg;

    QString typeString() const;
    // TODO: add context perhaps..
  };

  LogHandler(Context &context);
  virtual ~LogHandler();

  /// Message handler to be installed via \p qInstallMessageHandler().
  static void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg);

  void addEntry(const Entry &entry);
  QList<Entry> entries() const;

signals:
  void newEntry(const Entry &entry);

private:
  QList<Entry> entries_;
};

} // namespace dispar

#endif // SRC_LOGHANDLER_H
