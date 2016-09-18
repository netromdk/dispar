#ifndef DISPAR_BINARY_WIDGET_H
#define DISPAR_BINARY_WIDGET_H

#include <QHash>
#include <QWidget>

#include "../formats/Format.h"

class QListWidget;
class QTextEdit;
class QTextDocument;

class BinaryWidget : public QWidget {
  Q_OBJECT

public:
  BinaryWidget(std::shared_ptr<Format> fmt);

  QString file() const;

  // void commit();

signals:
  // void modified();

private slots:
  void onSymbolChosen(int row);
  void onCursorPositionChanged();

private:
  void createLayout();
  void setup();

  std::shared_ptr<Format> fmt;

  QListWidget *symbolList;
  QTextEdit *mainView;
  QTextDocument *doc;
  QHash<quint64, int> offsetBlock; // offset -> block
};

#endif // DISPAR_BINARY_WIDGET_H
