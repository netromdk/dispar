#ifndef DISPAR_PERSISTENT_SPLITTER_H
#define DISPAR_PERSISTENT_SPLITTER_H

#include <QSplitter>
#include <QString>

namespace dispar {

class PersistentSplitter : public QSplitter {
public:
  PersistentSplitter(const QString &settingsKey, Qt::Orientation = Qt::Horizontal,
                     QWidget *parent = nullptr);
  ~PersistentSplitter() override;

  PersistentSplitter(const PersistentSplitter &other) = delete;
  PersistentSplitter &operator=(const PersistentSplitter &rhs) = delete;

  PersistentSplitter(PersistentSplitter &&other) = delete;
  PersistentSplitter &operator=(PersistentSplitter &&rhs) = delete;

protected:
  void showEvent(QShowEvent *event) override;

private:
  QString settingsKey;
};

} // namespace dispar

#endif // DISPAR_PERSISTENT_SPLITTER_H
