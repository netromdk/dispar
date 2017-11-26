#ifndef DISPAR_FORMAT_LOADER_H
#define DISPAR_FORMAT_LOADER_H

#include <QThread>

#include <memory>

namespace dispar {

class Format;

class FormatLoader : public QThread {
  Q_OBJECT

public:
  FormatLoader(const QString &file);

signals:
  void failed(const QString &msg);
  void status(const QString &msg);
  void progress(float progress);
  void success(std::shared_ptr<Format> fmt);

protected:
  void run() override;

private:
  QString file;
};

} // namespace dispar

#endif // DISPAR_FORMAT_LOADER_H
