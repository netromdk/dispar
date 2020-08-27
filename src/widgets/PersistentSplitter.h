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

protected:
  void showEvent(QShowEvent *event) override;

private:
  QString settingsKey;
};

} // namespace dispar

#endif // DISPAR_PERSISTENT_SPLITTER_H
