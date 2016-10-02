#ifndef DISPAR_BINARY_WIDGET_H
#define DISPAR_BINARY_WIDGET_H

#include <QHash>
#include <QWidget>

#include "../BinaryObject.h"

class TagsEdit;

class QLabel;
class QListWidget;
class QPlainTextEdit;
class QTextDocument;

class BinaryWidget : public QWidget {
  Q_OBJECT

public:
  BinaryWidget(std::shared_ptr<BinaryObject> &object);

  // void commit();

signals:
  // void modified();

protected:
  void showEvent(QShowEvent *event);

private slots:
  void onSymbolChosen(int row);
  void onCursorPositionChanged();
  void onShowMachineCodeChanged(bool show);

  void filterSymbols(const QString &filter);

private:
  void createLayout();
  void setup();

  std::shared_ptr<BinaryObject> object;

  bool shown;
  QListWidget *symbolList, *stringList;
  QPlainTextEdit *mainView;
  QTextDocument *doc;
  QHash<quint64, int> offsetBlock; // offset -> block
  QList<int> codeBlocks;
  QLabel *addressLabel, *offsetLabel, *machineCodeLabel, *binaryLabel, *sizeLabel, *archLabel,
    *fileTypeLabel;
  TagsEdit *tagsEdit;
};

#endif // DISPAR_BINARY_WIDGET_H
