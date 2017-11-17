#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QFileInfo>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMenu>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProgressDialog>
#include <QPushButton>
#include <QTabWidget>
#include <QTextBlockUserData>
#include <QTimer>

#include "BinaryObject.h"
#include "CStringReader.h"
#include "Context.h"
#include "Project.h"
#include "Util.h"
#include "widgets/BinaryWidget.h"
#include "widgets/DisassemblerDialog.h"
#include "widgets/PersistentSplitter.h"
#include "widgets/TagsEdit.h"
#include "widgets/ToggleBox.h"

namespace {

// Temporary solution!
class TextBlockUserData : public QTextBlockUserData {
public:
  quint64 address = 0, offset = 0;
  int addressStart = 0, addressEnd = 0;
  int bytesStart = 0, bytesEnd = 0;
  QString bytes;
};

} // namespace

BinaryWidget::BinaryWidget(BinaryObject *object) : object(object), shown(false), doc(nullptr)
{
  Q_ASSERT(object);
  createLayout();

  auto &ctx = Context::get();
  connect(&ctx, &Context::showMachineCodeChanged, this, &BinaryWidget::onShowMachineCodeChanged);
}

void BinaryWidget::showEvent(QShowEvent *event)
{
  if (!shown) {
    shown = true;
    setup();
  }
}

bool BinaryWidget::eventFilter(QObject *obj, QEvent *event)
{
  if (obj == tagList && event->type() == QEvent::KeyPress) {
    auto *keyEvent = static_cast<QKeyEvent *>(event);
    switch (keyEvent->key()) {
    case Qt::Key_Backspace:
    case Qt::Key_Delete:
      removeSelectedTags();
      break;
    }
    return true;
  }

  return QWidget::eventFilter(obj, event);
}

void BinaryWidget::onSymbolChosen(int row)
{
  auto *list = qobject_cast<QListWidget *>(sender());

  // If offset is found then put the cursor at that line.
  auto *item = list->item(row);
  if (!item) return;

  auto offset = item->data(Qt::UserRole).toLongLong();
  selectAddress(offset);
}

void BinaryWidget::onCursorPositionChanged()
{
  auto cursor = mainView->textCursor();

  // Mark the whole line to highlight it.
  auto highlight = mainView->palette().highlight();
  highlight.setColor(highlight.color().lighter(120));
  QTextEdit::ExtraSelection selection;
  selection.format.setBackground(highlight);
  selection.format.setProperty(QTextFormat::FullWidthSelection, true);
  selection.cursor = cursor;
  mainView->setExtraSelections({selection});

  auto block = cursor.block();
  auto *userData = dynamic_cast<TextBlockUserData *>(block.userData());
  if (userData) {
    addressLabel->setText(
      tr("Address: 0x%1 (%2)").arg(userData->address, 0, 16).arg(userData->address));
    offsetLabel->setText(
      tr("Offset: 0x%1 (%2)").arg(userData->offset, 0, 16).arg(userData->offset));

    if (!userData->bytes.isEmpty()) {
      machineCodeLabel->setText(tr("Assembly: %1").arg(userData->bytes));
      machineCodeLabel->show();
    }
    else {
      machineCodeLabel->hide();
    }

    tagsEdit->setAddress(userData->address);
  }
}

void BinaryWidget::onShowMachineCodeChanged(bool show)
{
  QProgressDialog progDiag(this);
  progDiag.setWindowFlags(progDiag.windowFlags() | Qt::WindowStaysOnTopHint);
  progDiag.setLabelText(tr("Modifying machine code visibility.."));
  progDiag.setCancelButton(nullptr);
  progDiag.setRange(0, 0);
  progDiag.show();
  qApp->processEvents();
  qDebug() << qPrintable(progDiag.labelText());

  QElapsedTimer elapsedTimer;
  elapsedTimer.start();

  auto cursor = mainView->textCursor();
  cursor.beginEditBlock();

  auto block = doc->findBlockByNumber(codeBlocks.first());
  while (block.isValid()) {
    auto *userData = dynamic_cast<TextBlockUserData *>(block.userData());
    if (userData && !userData->bytes.isEmpty()) {
      cursor.setPosition(block.position() + userData->bytesStart - 1);
      if (show && userData->bytesEnd == -1) {
        cursor.insertText(QString("%1").arg(userData->bytes, -24));
        userData->bytesEnd = userData->bytesStart + 24;
      }
      else if (!show && userData->bytesEnd != -1) {
        cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, 24);
        cursor.removeSelectedText();
        userData->bytesEnd = -1;
      }
    }

    block = block.next();
  }

  cursor.endEditBlock();

  qDebug() << "Modified machine code visibility in" << elapsedTimer.restart() << "ms";
}

void BinaryWidget::onCustomContextMenuRequested(const QPoint &pos)
{
  const auto selected = mainView->textCursor().selectedText();
  if (selected.isEmpty()) return;

  // See if selection is convertible to a number, possibly an address.
  bool isAddress = false;
  const auto address = Util::convertAddress(selected, &isAddress);

  QMenu menu(mainView);

  menu.addAction(tr("Copy"), this, [&selected] { QApplication::clipboard()->setText(selected); },
                 QKeySequence::Copy);

  menu.addSeparator();

  menu.addAction(tr("Disassemble"), this, [this, &selected] {
    DisassemblerDialog diag(this, object->cpuType(), selected);
    diag.exec();
  });

  auto *jumpAction =
    menu.addAction(tr("Jump to Address"), this, [this, address] { selectAddress(address); });
  jumpAction->setEnabled(isAddress);

  menu.exec(mainView->mapToGlobal(pos));
}

void BinaryWidget::filterSymbols(const QString &filter)
{
  QListWidget *list = nullptr;
  if (symbolList->isVisible()) {
    list = symbolList;
  }
  else if (stringList->isVisible()) {
    list = stringList;
  }
  else {
    list = tagList;
  }

  // Unhide all.
  auto allItems = list->findItems("*", Qt::MatchWildcard);
  for (auto *item : allItems) {
    item->setHidden(false);
  }

  auto matches = list->findItems(filter, Qt::MatchContains);

  // Hide all that are not matches.
  for (auto *item : allItems) {
    if (!matches.contains(item)) {
      item->setHidden(true);
    }
  }
}

void BinaryWidget::createLayout()
{
  auto &ctx = Context::get();
  auto *project = ctx.project();
  Q_ASSERT(project);

  // Symbols left bar.

  symbolList = new QListWidget;
  symbolList->setSortingEnabled(true);
  connect(symbolList, &QListWidget::currentRowChanged, this, &BinaryWidget::onSymbolChosen);

  stringList = new QListWidget;
  stringList->setSortingEnabled(true);
  connect(stringList, &QListWidget::currentRowChanged, this, &BinaryWidget::onSymbolChosen);

  tagList = new QListWidget;
  tagList->setSortingEnabled(true);
  tagList->setSelectionMode(QAbstractItemView::ExtendedSelection);
  tagList->installEventFilter(this);
  connect(tagList, &QListWidget::currentRowChanged, this, &BinaryWidget::onSymbolChosen);

  connect(project, &Project::tagsChanged, this, &BinaryWidget::updateTagList);
  updateTagList();

  auto *tabWidget = new QTabWidget;
  tabWidget->addTab(symbolList, tr("Functions"));
  tabWidget->addTab(stringList, tr("Strings"));
  tabWidget->addTab(tagList, tr("Tags"));

  auto *filterSymLine = new QLineEdit;
  filterSymLine->setPlaceholderText(tr("Filter symbols.."));

  connect(tabWidget, &QTabWidget::currentChanged, this,
          [filterSymLine](int index) { filterSymLine->clear(); });

  connect(filterSymLine, &QLineEdit::textEdited, this, &BinaryWidget::filterSymbols);

  auto *symbolsLayout = new QVBoxLayout;
  symbolsLayout->setContentsMargins(0, 0, 0, 0);
  symbolsLayout->addWidget(tabWidget);
  symbolsLayout->addWidget(filterSymLine);

  auto *symbolsWidget = new QWidget;
  symbolsWidget->setLayout(symbolsLayout);

  // Position right bar.

  addressLabel = new QLabel;
  offsetLabel = new QLabel;
  machineCodeLabel = new QLabel;

  auto *positionLayout = new QVBoxLayout;
  positionLayout->setContentsMargins(5, 5, 5, 5);
  positionLayout->addWidget(addressLabel);
  positionLayout->addWidget(offsetLabel);
  positionLayout->addWidget(machineCodeLabel);

  auto *positionBox = new ToggleBox(tr("Position"), "BinaryWidget.positionBox");
  positionBox->setContentLayout(positionLayout);
  positionBox->setExpanded();

  auto binaryFile = project->binary();
  auto binarySize = QFileInfo(binaryFile).size();

  binaryLabel = new QLabel;
  binaryLabel->setWordWrap(true);

  QFontMetrics metrics(binaryLabel->font());
  auto elidedText = metrics.elidedText(binaryFile, Qt::ElideLeft, 200);
  binaryLabel->setText(tr("File: %1").arg(elidedText));
  binaryLabel->setToolTip(binaryFile);

  sizeLabel = new QLabel(tr("Size: %1").arg(Util::formatSize(binarySize)));

  archLabel = new QLabel(
    tr("Arch: %1 %2").arg(cpuTypeName(object->cpuType())).arg(cpuTypeName(object->cpuSubType())));

  fileTypeLabel = new QLabel(tr("Type: %1").arg(fileTypeName(object->fileType())));

  auto *binaryOpenFolderButton = new QPushButton(tr("Open Folder"));
  binaryOpenFolderButton->setToolTip(tr("Open folder of binary file."));

  connect(binaryOpenFolderButton, &QPushButton::clicked, this, [binaryFile] {
    auto folder = QFileInfo(binaryFile).dir().absolutePath();
    QDesktopServices::openUrl(QUrl::fromLocalFile(folder));
  });

  auto *binaryButtonLayout = new QHBoxLayout;
  binaryButtonLayout->setContentsMargins(0, 0, 0, 0);
  binaryButtonLayout->addStretch();
  binaryButtonLayout->addWidget(binaryOpenFolderButton);
  binaryButtonLayout->addStretch();

  auto *binaryLayout = new QVBoxLayout;
  binaryLayout->setContentsMargins(5, 5, 5, 5);
  binaryLayout->addWidget(binaryLabel);
  binaryLayout->addWidget(sizeLabel);
  binaryLayout->addWidget(archLabel);
  binaryLayout->addWidget(fileTypeLabel);
  binaryLayout->addLayout(binaryButtonLayout);

  auto *binaryBox = new ToggleBox(tr("Binary"), "BinaryWidget.binaryBox");
  binaryBox->setContentLayout(binaryLayout);
  binaryBox->setExpanded();

  tagsEdit = new TagsEdit;

  auto *tagsLayout = new QVBoxLayout;
  tagsLayout->setContentsMargins(5, 5, 5, 5);
  tagsLayout->addWidget(tagsEdit);

  auto *tagsBox = new ToggleBox(tr("Tags"), "BinaryWidget.tagsBox");
  tagsBox->setContentLayout(tagsLayout);
  tagsBox->setMaximumHeight(150);
  tagsBox->setExpanded();

  auto *propertiesLayout = new QVBoxLayout;
  propertiesLayout->setContentsMargins(0, 0, 0, 0);
  propertiesLayout->addWidget(positionBox);
  propertiesLayout->addWidget(binaryBox);
  propertiesLayout->addWidget(tagsBox);
  propertiesLayout->addStretch();

  auto *propertiesWidget = new QWidget;
  propertiesWidget->setLayout(propertiesLayout);

  // Main center view.

  mainView = new QPlainTextEdit;
  mainView->setReadOnly(true);
  mainView->setCenterOnScroll(true);
  mainView->setLineWrapMode(QPlainTextEdit::NoWrap);
  mainView->setContextMenuPolicy(Qt::CustomContextMenu);
  mainView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
  connect(mainView, &QPlainTextEdit::cursorPositionChanged, this,
          &BinaryWidget::onCursorPositionChanged);
  connect(mainView, &QWidget::customContextMenuRequested, this,
          &BinaryWidget::onCustomContextMenuRequested);

  doc = mainView->document();

  auto *vertSplitter = new PersistentSplitter("BinaryWidget.vertSplitter");
  vertSplitter->addWidget(symbolsWidget);
  vertSplitter->addWidget(mainView);
  vertSplitter->addWidget(propertiesWidget);

  vertSplitter->setSizes(QList<int>{200, 600, 200});

  auto *layout = new QHBoxLayout;
  layout->setContentsMargins(5, 5, 5, 5);
  layout->addWidget(vertSplitter);

  setLayout(layout);
}

void BinaryWidget::setup()
{
  QElapsedTimer elapsedTimer;
  elapsedTimer.start();

  QProgressDialog setupDiag(this);
  setupDiag.setLabelText(tr("Setting up for binary data.."));
  setupDiag.setCancelButton(nullptr);
  setupDiag.setRange(0, 4);
  setupDiag.show();
  qApp->processEvents();
  qDebug() << qPrintable(setupDiag.labelText());

  auto symbols = object->symbolTable().symbols();
  Util::copyTo(object->dynSymbolTable().symbols(), symbols);

  // Create text edit of all binary contents.
  QTextCursor cursor(doc);

  auto &ctx = Context::get();

  auto appendInstruction = [this, &cursor, &ctx](quint64 address, quint64 offset,
                                                 const QString &bytes, const QString &instruction,
                                                 const QString &operands) {
    auto *userData = new TextBlockUserData;
    userData->address = address;
    userData->addressStart = 0;
    userData->addressEnd = 20;

    userData->offset = offset;

    bool smc = ctx.showMachineCode();
    userData->bytes = bytes;
    userData->bytesStart = userData->addressEnd + 1;
    userData->bytesEnd = (smc ? userData->bytesStart + 24 : -1);

    cursor.insertBlock();
    cursor.insertText(QString("%1%2%3%4")
                        .arg(QString("0x%1").arg(address, 0, 16), -20)
                        .arg(smc ? QString("%1").arg(bytes, -24) : QString())
                        .arg(instruction, -10)
                        .arg(operands));

    auto block = cursor.block();
    block.setUserData(userData);

    offsetBlock[userData->address] = block.blockNumber();
    codeBlocks << block.blockNumber();
  };

  auto appendString = [this, &cursor](quint64 address, quint64 offset, const QString &string) {
    cursor.insertBlock();
    cursor.insertText(QString("%1%2%3")
                        .arg(QString("0x%1").arg(address, 0, 16), -20)
                        .arg(QString("\"%1\"").arg(string))
                        .arg(tr("; size=%1").arg(string.size()), 11));

    auto *userData = new TextBlockUserData;
    userData->address = address;
    userData->offset = offset;

    auto block = cursor.block();
    block.setUserData(userData);

    offsetBlock[userData->address] = block.blockNumber();
  };

  const auto presetupTime = elapsedTimer.restart();
  qDebug() << presetupTime << "ms";

  cursor.beginEditBlock();

  setupDiag.setValue(1);
  setupDiag.setLabelText(tr("Generating UI for disassembled sections.."));
  qApp->processEvents();
  qDebug() << qPrintable(setupDiag.labelText());

  quint64 firstAddress = 0;
  for (auto *section : object->sections()) {
    auto disasm = section->disassembly();
    if (!disasm) continue;

    cursor.movePosition(QTextCursor::End);

    // There is a default block at the beginning so reuse that.
    if (cursor.block() != doc->firstBlock()) {
      cursor.insertBlock();
    }

    auto secName = Section::typeName(section->type());
    cursor.insertText("===== " + secName + " =====");

    for (size_t i = 0; i < disasm->count(); i++) {
      auto *instr = disasm->instructions(i);
      auto offset = instr->address;
      auto addr = offset + section->address();

      // Check if address is the start of a procedure.
      for (const auto &symbol : symbols) {
        if (symbol.value() == addr && !symbol.string().isEmpty()) {
          cursor.movePosition(QTextCursor::End);
          cursor.insertBlock();
          cursor.insertText("\nPROC: " + Util::demangle(symbol.string()) + "\n");
          break;
        }
      }

      if (firstAddress == 0) {
        firstAddress = addr;
      }

      appendInstruction(addr, offset, Util::bytesToHex(instr->bytes, instr->size), instr->mnemonic,
                        instr->op_str);
    }

    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock();
    cursor.insertText("\n===== /" + secName + " =====\n");
  }

  const auto disSectionsTime = elapsedTimer.restart();
  qDebug() << disSectionsTime << "ms";

  setupDiag.setValue(2);
  setupDiag.setLabelText(tr("Generating UI for string sections.."));
  qApp->processEvents();
  qDebug() << qPrintable(setupDiag.labelText());

  // Show cstring+string sections.
  auto stringSecs = object->sectionsByType(Section::Type::CSTRING);
  stringSecs << object->sectionsByType(Section::Type::STRING);
  for (auto *section : stringSecs) {
    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock();
    auto secName = Section::typeName(section->type());
    cursor.insertText("===== " + secName + " =====\n");

    CStringReader reader(section->data());
    while (reader.next()) {
      auto offset = reader.offset();
      auto addr = offset + section->address();
      auto string = reader.string();

      appendString(addr, offset, string);

      addSymbolToList(reader.string(), addr, stringList);
    }

    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock();
    cursor.insertText("\n===== /" + secName + " =====\n");
  }

  const auto stringSectionsTime = elapsedTimer.restart();
  qDebug() << stringSectionsTime << "ms";

  setupDiag.setValue(3);
  setupDiag.setLabelText(tr("Generating sidebar with functions and strings.."));
  qApp->processEvents();
  qDebug() << qPrintable(setupDiag.labelText());

  // Fill side bar with function names of the symbol tables.
  for (const auto &symbol : symbols) {
    if (symbol.value() == 0) continue;

    auto func = Util::demangle(symbol.string());
    if (func.isEmpty()) {
      func = QString("unnamed_%1").arg(symbol.value(), 0, 16);
    }
    if (offsetBlock.contains(symbol.value())) {
      func += " *";
    }

    addSymbolToList(func, symbol.value() /* offset to symbol */, symbolList);
  }

  const auto sidebarTime = elapsedTimer.restart();
  qDebug() << sidebarTime << "ms";

  cursor.endEditBlock();
  setupDiag.setValue(4);

  Util::scrollToTop(mainView);

  const auto totalTime =
    presetupTime + disSectionsTime + stringSectionsTime + sidebarTime + elapsedTimer.restart();
  qDebug() << "Setup in" << totalTime << "ms";

  if (!offsetBlock.isEmpty()) {
    selectAddress(firstAddress);
  }
}

void BinaryWidget::updateTagList()
{
  tagList->clear();

  const auto &tags = Context::get().project()->tags();
  for (const auto addr : tags.keys()) {
    for (const auto &tag : tags[addr]) {
      addSymbolToList(tag, addr, tagList);
    }
  }
}

void BinaryWidget::addSymbolToList(const QString &text, quint64 address, QListWidget *list)
{
  class ListWidgetItem : public QListWidgetItem {
  public:
    bool operator<(const QListWidgetItem &other) const override
    {
      auto addr1 = data(Qt::UserRole).toLongLong();
      auto addr2 = other.data(Qt::UserRole).toLongLong();

      // If same address then sort for tag lexically.
      if (addr1 == addr2) {
        return text() < other.text();
      }

      // Otherwise by address ascending.
      return addr1 < addr2;
    }
  };

  auto *item = new ListWidgetItem;
  item->setText(text);
  item->setData(Qt::UserRole, address);
  item->setToolTip(QString("0x%1").arg(address, 0, 16));
  list->addItem(item);
}

void BinaryWidget::selectAddress(quint64 address)
{
  if (!offsetBlock.contains(address)) {
    return;
  }

  auto blockNum = offsetBlock[address];
  auto block = doc->findBlockByNumber(blockNum);
  auto cursor = mainView->textCursor();
  cursor.setPosition(block.position());
  mainView->setTextCursor(cursor);
  mainView->ensureCursorVisible();
}

void BinaryWidget::removeSelectedTags()
{
  QStringList tags;
  for (auto *item : tagList->selectedItems()) {
    tags << item->text();
  }
  Context::get().project()->removeAddressTags(tags);
}
