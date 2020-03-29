#ifndef DISPAR_BINARY_WIDGET_H
#define DISPAR_BINARY_WIDGET_H

#include <QHash>
#include <QWidget>

#include "BinaryObject.h"

class QLabel;
class QListWidget;
class QPlainTextEdit;
class QTextDocument;

namespace dispar {

class TagsEdit;
class HexEditor;
class DisassemblyEditor;
class MacSdkVersionsEditor;

class BinaryWidget : public QWidget {
  Q_OBJECT

public:
  BinaryWidget(BinaryObject *object);
  virtual ~BinaryWidget();

signals:
  void modified();

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
  void selectBlock(int number);
  void removeSelectedTags();

  /// Check if section has different modifications than \p priorModifications and emit modified.
  /** It will also add to re-run setup() if modified. */
  void checkModified(const Section *section,
                     const QList<Section::ModifiedRegion> &priorModifications);

  BinaryObject *object;

  bool shown;
  QListWidget *symbolList, *stringList, *tagList;
  QList<QListWidget *> symbolLists;
  QHash<QListWidget *, QString> listFilters;
  QPlainTextEdit *mainView;
  QTextDocument *doc;
  QHash<quint64, int> offsetBlock;          ///< Offset -> block
  QHash<const Section *, int> sectionBlock; ///< Section -> block
  QList<int> codeBlocks;
  QLabel *addressLabel, *offsetLabel, *machineCodeLabel, *binaryLabel, *sizeLabel, *archLabel,
    *fileTypeLabel;
  TagsEdit *tagsEdit;
  QHash<Section *, DisassemblyEditor *> disassemblyEditors;
  QHash<Section *, MacSdkVersionsEditor *> macSdkVersionsEditors;
  QHash<Section *, HexEditor *> hexEditors;
};

} // namespace dispar

#endif // DISPAR_BINARY_WIDGET_H
