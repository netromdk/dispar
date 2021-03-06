#include <QMutex>
#include <QObject>
#include <QThreadPool>

namespace dispar {

class Section;

class AddrHexAsciiEncoderTask : public QObject, public QRunnable {
  Q_OBJECT

public:
  AddrHexAsciiEncoderTask(const QByteArray &data, quint64 address, quint64 index, quint64 size);

  void run() override;

signals:
  void result(quint64 index, const QString &data);

private:
  const QByteArray &data;
  const quint64 address, index, size;
};

class AddrHexAsciiEncoder : public QObject {
  Q_OBJECT

public:
  AddrHexAsciiEncoder(const Section *section);

  void start(bool blocking = false);
  QString result() const;

signals:
  void finished();

private slots:
  void addResult(quint64 index, const QString &result);

private:
  const Section *section;

  QThreadPool pool;
  QString result_;
  int tasks = 0;

  struct Result {
    quint64 index;
    QString data;
  };
  QList<Result> results;
  QMutex mutex;
};

} // namespace dispar
