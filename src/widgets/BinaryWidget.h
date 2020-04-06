#ifndef DISPAR_BINARY_WIDGET_H
#define DISPAR_BINARY_WIDGET_H

#include <QElapsedTimer>
#include <QHash>
#include <QPointer>
#include <QWidget>

#include <memory>

#include "BinaryObject.h"
#include "SymbolTable.h"

class QLabel;
class QTabWidget;
class QTextCursor;
class QListWidget;
class QPlainTextEdit;
class QTextDocument;
class QProgressDialog;

namespace dispar {

class Context;
class TagsEdit;
class HexEditor;
class DisassemblyEditor;
class MacSdkVersionsEditor;

class BinaryWidget : public QWidget {
  Q_OBJECT
  friend class OmniSearchDialog;

public:
  BinaryWidget(BinaryObject *object);
  virtual ~BinaryWidget();

  void reloadUi();

signals:
  void modified();
  void loaded();

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
  void selectSection(const Section *section);
  void removeSelectedTags();

  /// Check if section has different modifications than \p priorModifications and emit modified.
  /** It will also add to re-run setup() if modified. */
  void checkModified(const Section *section,
                     const QList<Section::ModifiedRegion> &priorModifications);

  BinaryObject *object_;

  Context &context;

  bool shown;
  QListWidget *symbolList_, *stringList_, *tagList_;
  QTabWidget *tabWidget;
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

  /// Setup related.
  //@{
  QPointer<QProgressDialog> setupDiag;
  QElapsedTimer setupElapsedTimer;
  std::unique_ptr<QTextCursor> setupCursor;
  QHash<quint64, QString> procNameMap;
  quint64 firstAddress = 0;
  SymbolTable::EntryList symbols;
  void appendInstruction(quint64 address, quint64 offset, const QString &bytes,
                         const QString &instruction, const QString &operands);
  void appendString(quint64 address, quint64 offset, const QString &string);
  qint64 presetup();
  qint64 setupDisassembledSections();
  qint64 setupStringSections();
  qint64 setupLoadCommandSections();
  qint64 setupMiscSections();
  qint64 setupSidebar();
  //@}
};

} // namespace dispar

#endif // DISPAR_BINARY_WIDGET_H
