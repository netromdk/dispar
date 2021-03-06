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
  ~BinaryWidget() override;

  BinaryWidget(const BinaryWidget &other) = delete;
  BinaryWidget &operator=(const BinaryWidget &rhs) = delete;

  BinaryWidget(BinaryWidget &&other) = delete;
  BinaryWidget &operator=(BinaryWidget &&rhs) = delete;

  void reloadUi();

signals:
  void modified();
  void loaded();

protected:
  void showEvent(QShowEvent *event) override;
  bool eventFilter(QObject *obj, QEvent *event) override;

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
  void selectPosition(int pos);
  void removeSelectedTags();

  /// Check if section has different modifications than \p priorModifications and emit modified.
  /** It will also add to re-run setup() if modified. */
  void checkModified(const Section *section,
                     const QList<Section::ModifiedRegion> &priorModifications);

  BinaryObject *object_;

  Context &context;

  bool shown = false;
  QListWidget *symbolList_ = nullptr, *stringList_ = nullptr, *tagList_ = nullptr;
  QTabWidget *tabWidget = nullptr;
  QList<QListWidget *> symbolLists;
  QHash<QListWidget *, QString> listFilters;
  QPlainTextEdit *mainView = nullptr;
  QTextDocument *doc = nullptr;
  QHash<quint64, int> offsetBlock;          ///< Offset -> block
  QHash<const Section *, int> sectionBlock; ///< Section -> block
  QList<int> codeBlocks;
  QLabel *addressLabel = nullptr, *offsetLabel = nullptr, *machineCodeLabel = nullptr,
         *binaryLabel = nullptr, *sizeLabel = nullptr, *archLabel = nullptr,
         *fileTypeLabel = nullptr;
  TagsEdit *tagsEdit = nullptr;
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
