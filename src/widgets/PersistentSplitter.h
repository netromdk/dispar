#ifndef DISPAR_PERSISTENT_SPLITTER_H
#define DISPAR_PERSISTENT_SPLITTER_H

#include <QSplitter>
#include <QString>

class PersistentSplitter : public QSplitter {
public:
  PersistentSplitter(const QString &settingsKey, Qt::Orientation = Qt::Horizontal,
                     QWidget *parent = nullptr);
  ~PersistentSplitter();

protected:
  void showEvent(QShowEvent *event);

private:
  QString settingsKey;
};

#endif // DISPAR_PERSISTENT_SPLITTER_H
