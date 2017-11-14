#ifndef DISPAR_BINARY_WIDGET_H
#define DISPAR_BINARY_WIDGET_H

#include <QHash>
#include <QWidget>

#include "BinaryObject.h"

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
  bool eventFilter(QObject *obj, QEvent *event);

private slots:
  void onSymbolChosen(int row);
  void onCursorPositionChanged();
  void onShowMachineCodeChanged(bool show);
  void onCustomContextMenuRequested(const QPoint &pos);

  void filterSymbols(const QString &filter);

private:
  void createLayout();
  void setup();
  void updateTagList();
  void addSymbolToList(const QString &text, quint64 address, QListWidget *list);
  void selectAddress(quint64 address);
  void removeSelectedTags();

  std::shared_ptr<BinaryObject> object;

  bool shown;
  QListWidget *symbolList, *stringList, *tagList;
  QPlainTextEdit *mainView;
  QTextDocument *doc;
  QHash<quint64, int> offsetBlock; // offset -> block
  QList<int> codeBlocks;
  QLabel *addressLabel, *offsetLabel, *machineCodeLabel, *binaryLabel, *sizeLabel, *archLabel,
    *fileTypeLabel;
  TagsEdit *tagsEdit;
};

#endif // DISPAR_BINARY_WIDGET_H
