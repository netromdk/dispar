#ifndef DISPAR_BINARY_WIDGET_H
#define DISPAR_BINARY_WIDGET_H

#include <QWidget>

#include "../formats/Format.h"

/*
class Pane;
class QListWidget;
class QStackedLayout;
*/

class BinaryWidget : public QWidget {
  Q_OBJECT

public:
  BinaryWidget(std::shared_ptr<Format> fmt);

  QString file() const
  {
    return fmt->file();
  }

  // void commit();

signals:
  // void modified();

private slots:
  // void onModeChanged(int row);

private:
  // void createLayout();
  // void setup();
  // void addPane(const QString &title, Pane *pane, int level = 0);

  std::shared_ptr<Format> fmt;

  /*
  QListWidget *listWidget;
  QStackedLayout *stackLayout;
  */
};

#endif // DISPAR_BINARY_WIDGET_H
