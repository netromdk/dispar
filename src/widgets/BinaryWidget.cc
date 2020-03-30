#include <QApplication>
#include <QBuffer>
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
#include <QSet>
#include <QTabWidget>
#include <QTextBlockUserData>
#include <QTimer>

#include <cassert>

#include "AddrHexAsciiEncoder.h"
#include "BinaryObject.h"
#include "CStringReader.h"
#include "Context.h"
#include "MacSdkVersionPatcher.h"
#include "Project.h"
#include "Reader.h"
#include "Util.h"
#include "cxx.h"
#include "widgets/BinaryWidget.h"
#include "widgets/DisassemblerDialog.h"
#include "widgets/DisassemblyEditor.h"
#include "widgets/HexEditor.h"
#include "widgets/MacSdkVersionsEditor.h"
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

namespace dispar {

BinaryWidget::BinaryWidget(BinaryObject *object_) : object(object_), shown(false), doc(nullptr)
{
  assert(object);
  createLayout();

  auto &ctx = Context::get();
  connect(&ctx, &Context::showMachineCodeChanged, this, &BinaryWidget::onShowMachineCodeChanged);
}

BinaryWidget::~BinaryWidget()
{
  qDeleteAll(disassemblyEditors.values());
  qDeleteAll(hexEditors.values());
  qDeleteAll(macSdkVersionsEditors.values());
}

void BinaryWidget::reloadUi()
{
  setup();
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
    const auto *keyEvent = static_cast<QKeyEvent *>(event);
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
  const auto *list = qobject_cast<QListWidget *>(sender());

  // If offset is found then put the cursor at that line.
  const auto *item = list->item(row);
  if (!item) return;

  auto offset = item->data(Qt::UserRole).toLongLong();
  selectAddress(offset);
}

void BinaryWidget::onCursorPositionChanged()
{
  const auto cursor = mainView->textCursor();

  // Mark the whole line to highlight it.
  auto highlight = mainView->palette().highlight();
  highlight.setColor(highlight.color().lighter(120));
  QTextEdit::ExtraSelection selection;
  selection.format.setBackground(highlight);
  selection.format.setProperty(QTextFormat::FullWidthSelection, true);
  selection.cursor = cursor;
  mainView->setExtraSelections({selection});

  const auto block = cursor.block();
  const auto *userData = dynamic_cast<TextBlockUserData *>(block.userData());
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
  QMenu menu(mainView);

  // Jump to sections.
  auto *sectionMenu = menu.addMenu(tr("Jump to Section"));
  auto sortedSections = sectionBlock.keys();
  cxx::sort(sortedSections, [](const auto *s1, const auto *s2) { return s1->type() < s2->type(); });
  for (const auto *section : sortedSections) {
    sectionMenu->addAction(section->toString(), this,
                           [this, section] { selectBlock(sectionBlock[section]); });
  }

  auto cursor = mainView->textCursor();
  const auto *userData = dynamic_cast<TextBlockUserData *>(cursor.block().userData());
  if (userData) {
    for (auto *section : object->sections()) {
      if (!section->hasAddress(userData->address)) {
        continue;
      }

      if (section->type() == Section::Type::TEXT ||
          section->type() == Section::Type::SYMBOL_STUBS) {
        menu.addAction(tr("Edit '%1'").arg(section->toString()), this, [this, section] {
          const auto priorModRegions = section->modifiedRegions();

          auto *editor = disassemblyEditors.value(section, nullptr);
          if (!editor) {
            editor = new DisassemblyEditor(section, object, this);
            disassemblyEditors[section] = editor;
          }

          editor->exec();
          checkModified(section, priorModRegions);
        });
      }

      else if (section->type() == Section::Type::LC_VERSION_MIN_MACOSX ||
               section->type() == Section::Type::LC_VERSION_MIN_IPHONEOS ||
               section->type() == Section::Type::LC_VERSION_MIN_WATCHOS ||
               section->type() == Section::Type::LC_VERSION_MIN_TVOS) {
        menu.addAction(tr("Edit versions '%1'").arg(section->toString()), this, [this, section] {
          auto *editor = macSdkVersionsEditors.value(section, nullptr);
          if (!editor) {
            editor = new MacSdkVersionsEditor(section, object, this);
            macSdkVersionsEditors[section] = editor;
          }

          if (QDialog::Accepted == editor->exec()) {
            checkModified(section, {});
          }
        });
      }

      menu.addAction(tr("Hex edit '%1'").arg(section->toString()), this, [this, section] {
        const auto priorModRegions = section->modifiedRegions();

        auto *editor = hexEditors.value(section, nullptr);
        if (!editor) {
          editor = new HexEditor(section, object, this);
          hexEditors[section] = editor;
        }

        editor->exec();
        checkModified(section, priorModRegions);
      });
    }
  }

  const auto selected = cursor.selectedText();
  if (!selected.isEmpty()) {
    // See if selection is convertible to a number, possibly an address.
    bool isAddress = false;
    const auto address = Util::convertAddress(selected, &isAddress);

    menu.addSeparator();
    menu.addAction(
      tr("Copy"), this, [&selected] { QApplication::clipboard()->setText(selected); },
      QKeySequence::Copy);

    menu.addSeparator();
    menu.addAction(tr("Disassemble"), this, [this, &selected] {
      DisassemblerDialog diag(this, object->cpuType(), selected);
      diag.exec();
    });

    auto *jumpAction =
      menu.addAction(tr("Jump to Address"), this, [this, address] { selectAddress(address); });
    jumpAction->setEnabled(isAddress);
  }

  menu.exec(mainView->mapToGlobal(pos));
}

void BinaryWidget::filterSymbols(const QString &filter)
{
  QElapsedTimer elapsedTimer;
  elapsedTimer.start();
  qDebug() << "Filter using:" << filter;

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

  // Disable sorting while filtering to improve speed.
  list->setSortingEnabled(false);

  // Remember filter used for this list.
  listFilters[list] = filter;

  qDebug() << " Hiding all elements";
  const auto allItems = list->findItems("*", Qt::MatchWildcard);
  for (auto *item : allItems) {
    item->setHidden(true);
  }
  const auto hideTime = elapsedTimer.restart();
  qDebug() << " >" << hideTime << "ms";

  qDebug() << " Find all matches";
  const auto matches = list->findItems(filter, Qt::MatchContains);
  const auto matchingTime = elapsedTimer.restart();
  qDebug() << " >" << matchingTime << "ms";

  // Hide all that are not matches.
  qDebug() << " Show all matches";
  for (auto *item : matches) {
    item->setHidden(false);
  }
  const auto hideNonMatchTime = elapsedTimer.restart();
  qDebug() << " >" << hideNonMatchTime << "ms";

  list->setSortingEnabled(true);

  const auto total = hideTime + matchingTime + hideNonMatchTime + elapsedTimer.restart();
  qDebug() << "Filter in" << total << "ms";
}

void BinaryWidget::createLayout()
{
  auto &ctx = Context::get();
  auto *project = ctx.project();
  assert(project);

  // Symbols left bar.

  symbolList = new QListWidget;
  symbolList->setSortingEnabled(false);
  symbolList->setUniformItemSizes(true);
  connect(symbolList, &QListWidget::currentRowChanged, this, &BinaryWidget::onSymbolChosen);

  stringList = new QListWidget;
  stringList->setSortingEnabled(false);
  stringList->setUniformItemSizes(true);
  connect(stringList, &QListWidget::currentRowChanged, this, &BinaryWidget::onSymbolChosen);

  tagList = new QListWidget;
  tagList->setSortingEnabled(false);
  tagList->setUniformItemSizes(true);
  tagList->setSelectionMode(QAbstractItemView::ExtendedSelection);
  tagList->installEventFilter(this);
  connect(tagList, &QListWidget::currentRowChanged, this, &BinaryWidget::onSymbolChosen);

  connect(project, &Project::tagsChanged, this, &BinaryWidget::updateTagList);
  updateTagList();

  auto *tabWidget = new QTabWidget;
  tabWidget->addTab(symbolList, tr("Functions"));
  tabWidget->addTab(stringList, tr("Strings"));
  tabWidget->addTab(tagList, tr("Tags"));

  symbolLists << symbolList << stringList << tagList;
  assert(symbolLists.size() == 3);

  auto *filterSymLine = new QLineEdit;
  filterSymLine->setPlaceholderText(tr("Filter symbols.."));

  connect(tabWidget, &QTabWidget::currentChanged, this, [this, filterSymLine](int index) {
    filterSymLine->setText(listFilters.value(symbolLists[index]));
  });

  auto *filterButton = new QPushButton(tr("Filter"));
  connect(filterButton, &QPushButton::clicked, this,
          [this, filterSymLine] { filterSymbols(filterSymLine->text()); });

  connect(filterSymLine, &QLineEdit::returnPressed, filterButton, &QPushButton::click);

  auto *filterLayout = new QHBoxLayout;
  filterLayout->setContentsMargins(0, 0, 0, 0);
  filterLayout->addWidget(filterSymLine);
  filterLayout->addWidget(filterButton);

  auto *symbolsLayout = new QVBoxLayout;
  symbolsLayout->setContentsMargins(0, 0, 0, 0);
  symbolsLayout->addWidget(tabWidget);
  symbolsLayout->addLayout(filterLayout);

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
    tr("Arch: %1, %2").arg(cpuTypeName(object->cpuType())).arg(cpuTypeName(object->cpuSubType())));

  fileTypeLabel = new QLabel(tr("Type: %1").arg(fileTypeName(object->fileType())));

  auto *binaryOpenFolderButton = new QPushButton(tr("Open Folder"));
  binaryOpenFolderButton->setToolTip(tr("Open folder of binary file."));

  connect(binaryOpenFolderButton, &QPushButton::clicked, this, [binaryFile] {
    auto folder = QFileInfo(binaryFile).dir().absolutePath();
    QDesktopServices::openUrl(QUrl::fromLocalFile(folder));
  });

  auto *binaryRunDebuggerButton = new QPushButton(tr("Run In Debugger"));
  binaryRunDebuggerButton->setToolTip(tr("Run binary in debugger."));

  connect(binaryRunDebuggerButton, &QPushButton::clicked, this, [this, binaryFile] {
    const auto dbg = Context::get().debugger();
    if (!dbg.valid()) {
      QMessageBox::warning(this, "",
                           tr("No valid debugger is specified! Do so in the Options Dialog."));
      return;
    }
    if (!dbg.runnable()) {
      QMessageBox::warning(this, "", tr("Could not run debugger \"%1\"!").arg(dbg.program()));
      return;
    }
    dbg.detachStart(binaryFile);
  });

  auto *binaryButtonLayout = new QHBoxLayout;
  binaryButtonLayout->setContentsMargins(0, 0, 0, 0);
  binaryButtonLayout->addStretch();
  binaryButtonLayout->addWidget(binaryOpenFolderButton);
  binaryButtonLayout->addWidget(binaryRunDebuggerButton);
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

  // Make sure we start from a clean slate.
  mainView->clear();

  symbolList->clear();
  symbolList->setEnabled(false);

  stringList->clear();
  stringList->setEnabled(false);

  tagList->clear();
  tagList->setEnabled(false);

  offsetBlock.clear();
  sectionBlock.clear();
  codeBlocks.clear();

  QProgressDialog setupDiag(this);
  setupDiag.setLabelText(tr("Setting up for binary data.."));
  setupDiag.setCancelButton(nullptr);
  setupDiag.setRange(0, 6);
  setupDiag.show();
  qApp->processEvents();
  qDebug() << qPrintable(setupDiag.labelText());

  auto symbols = object->symbolTable().symbols();
  Util::copyTo(object->dynSymbolTable().symbols(), symbols);

  // Create temporary procedure name lookup map.
  QHash<quint64, QString> procNameMap;
  for (const auto &symbol : symbols) {
    if (!symbol.string().isEmpty()) {
      procNameMap[symbol.value()] = Util::demangle(symbol.string());
    }
  }

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
                        .arg(QString("\"%1\"").arg(Util::escapeWhitespace(string)))
                        .arg(tr("; size=%1").arg(string.size()), 11));

    auto *userData = new TextBlockUserData;
    userData->address = address;
    userData->offset = offset;

    auto block = cursor.block();
    block.setUserData(userData);

    offsetBlock[userData->address] = block.blockNumber();
  };

  const auto presetupTime = elapsedTimer.restart();
  qDebug() << ">" << presetupTime << "ms";

  cursor.beginEditBlock();

  setupDiag.setValue(1);
  setupDiag.setLabelText(tr("Generating UI for disassembled sections.."));
  qApp->processEvents();
  qDebug() << qPrintable(setupDiag.labelText());

  quint64 firstAddress = 0;
  for (auto *section : object->sections()) {
    const auto *disasm = section->disassembly();
    if (!disasm) continue;

    cursor.movePosition(QTextCursor::End);

    // There is a default block at the beginning so reuse that.
    if (cursor.block() != doc->firstBlock()) {
      cursor.insertBlock();
    }

    // Save section to block number.
    sectionBlock[section] = cursor.blockNumber();

    const auto secName = section->toString();
    qDebug() << "" << secName << "section..";
    cursor.insertText("===== " + secName + " =====");

    QElapsedTimer sectionTimer;
    sectionTimer.start();

    for (std::size_t i = 0; i < disasm->count(); i++) {
      const auto *instr = disasm->instructions(i);
      const auto offset = instr->address;
      const auto addr = offset + section->address();

      // Check if address is the start of a procedure.
      const auto it = procNameMap.find(addr);
      if (it != procNameMap.end()) {
        cursor.insertBlock();
        cursor.insertText("\nPROC: " + *it + "\n");
      }

      if (firstAddress == 0) {
        firstAddress = addr;
      }

      appendInstruction(addr, offset, Util::bytesToHex(instr->bytes, instr->size), instr->mnemonic,
                        instr->op_str);
    }

    qDebug() << " >" << sectionTimer.restart() << "ms";

    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock();
    cursor.insertText("\n===== /" + secName + " =====\n");
  }

  const auto disSectionsTime = elapsedTimer.restart();
  qDebug() << ">" << disSectionsTime << "ms";

  setupDiag.setValue(2);
  setupDiag.setLabelText(tr("Generating UI for string sections.."));
  qApp->processEvents();
  qDebug() << qPrintable(setupDiag.labelText());

  // Show cstring+string sections.
  for (auto *section : object->sectionsByTypes({Section::Type::CSTRING, Section::Type::STRING})) {
    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock();

    // Save section to block number.
    sectionBlock[section] = cursor.blockNumber();

    const auto secName = section->toString();
    qDebug() << "" << secName << "section..";
    cursor.insertText("===== " + secName + " =====\n");

    QElapsedTimer sectionTimer;
    sectionTimer.start();

    CStringReader reader(section->data());
    while (reader.next()) {
      const auto offset = reader.offset();
      const auto addr = offset + section->address();
      const auto string = reader.string();
      appendString(addr, offset, string);
      addSymbolToList(string, addr, stringList);
    }

    qDebug() << " >" << sectionTimer.restart() << "ms";

    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock();
    cursor.insertText("\n===== /" + secName + " =====\n");
  }

  const auto stringSectionsTime = elapsedTimer.restart();
  qDebug() << ">" << stringSectionsTime << "ms";

  setupDiag.setValue(3);
  setupDiag.setLabelText(tr("Generating UI for load commands.."));
  qApp->processEvents();
  qDebug() << qPrintable(setupDiag.labelText());

  // Show load command sections.
  for (auto *section : object->sectionsByTypes(
         {Section::Type::LC_VERSION_MIN_MACOSX, Section::Type::LC_VERSION_MIN_IPHONEOS,
          Section::Type::LC_VERSION_MIN_WATCHOS, Section::Type::LC_VERSION_MIN_TVOS})) {
    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock();

    // Save section to block number.
    sectionBlock[section] = cursor.blockNumber();

    const auto secName = section->toString();
    qDebug() << "" << secName << "section..";
    cursor.insertText("===== " + secName + " =====\n");

    QElapsedTimer sectionTimer;
    sectionTimer.start();

    MacSdkVersionPatcher patcher(*section);
    if (patcher.valid()) {
      static const auto versionString = [](const std::tuple<int, int> &version) {
        return QString("%1.%2").arg(std::get<0>(version)).arg(std::get<1>(version));
      };

      const auto target = patcher.target();
      const auto targetStr = QString("0x%1 (target %2)")
                               .arg(Util::encodeMacSdkVersion(target), 0, 16)
                               .arg(versionString(target));
      auto addr = section->address();
      appendString(addr, addr - section->address(), targetStr);

      const auto sdk = patcher.sdk();
      const auto sdkStr =
        QString("0x%1 (sdk %2)").arg(Util::encodeMacSdkVersion(sdk), 0, 16).arg(versionString(sdk));
      addr = section->address() + 4;
      appendString(addr, addr - section->address(), sdkStr);
    }

    qDebug() << " >" << sectionTimer.restart() << "ms";

    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock();
    cursor.insertText("\n===== /" + secName + " =====\n");
  }

  const auto lcSectionsTime = elapsedTimer.restart();
  qDebug() << ">" << lcSectionsTime << "ms";

  setupDiag.setValue(4);
  setupDiag.setLabelText(tr("Generating UI for miscellaneous sections.."));
  qApp->processEvents();
  qDebug() << qPrintable(setupDiag.labelText());

  // Show miscellaneous sections. The section not shown in specific ways will be address-hex-ASCII
  // encoded just to give some representation.
  for (auto *section : object->sectionsByTypes(
         {Section::Type::FUNC_STARTS, Section::Type::SYMBOLS, Section::Type::DYN_SYMBOLS,
          Section::Type::SYMBOL_STUBS, Section::Type::CODE_SIG})) {
    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock();

    // Save section to block number.
    sectionBlock[section] = cursor.blockNumber();

    const auto secName = section->toString();
    qDebug() << "" << secName << "section..";
    cursor.insertText("===== " + secName + " =====\n");

    QElapsedTimer sectionTimer;
    sectionTimer.start();

    AddrHexAsciiEncoder encoder(section);
    const bool blocking(true);
    encoder.start(blocking);
    const auto lines = encoder.result().split("\n");

    for (const auto &line : lines) {
      const auto pos = line.indexOf(':');
      if (pos == -1) continue;

      const auto addr = line.mid(0, pos).toULongLong(nullptr, 16);

      cursor.insertBlock();
      cursor.insertText(line);

      auto *userData = new TextBlockUserData;
      userData->address = addr;
      userData->offset = addr - section->address();

      auto block = cursor.block();
      block.setUserData(userData);

      offsetBlock[userData->address] = block.blockNumber();
    }

    qDebug() << " >" << sectionTimer.restart() << "ms";

    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock();
    cursor.insertText("\n===== /" + secName + " =====\n");
  }

  const auto miscSectionsTime = elapsedTimer.restart();
  qDebug() << ">" << miscSectionsTime << "ms";

  setupDiag.setValue(5);
  setupDiag.setLabelText(tr("Generating sidebar with functions and strings.."));
  qApp->processEvents();
  qDebug() << qPrintable(setupDiag.labelText());

  // Fill side bar with function names of the symbol tables.
  QSet<quint64> seenSymbols;
  for (const auto &symbol : symbols) {
    if (seenSymbols.contains(symbol.value())) {
      continue;
    }

    seenSymbols << symbol.value();

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
  qDebug() << ">" << sidebarTime << "ms";

  cursor.endEditBlock();
  setupDiag.setValue(6);

  Util::scrollToTop(mainView);

  const auto totalTime = presetupTime + disSectionsTime + stringSectionsTime + lcSectionsTime +
                         miscSectionsTime + sidebarTime + elapsedTimer.restart();
  qDebug() << "Setup in" << totalTime << "ms";

  symbolList->setSortingEnabled(true);
  symbolList->setEnabled(true);

  stringList->setSortingEnabled(true);
  stringList->setEnabled(true);

  tagList->setSortingEnabled(true);
  tagList->setEnabled(true);

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
      const auto addr1 = data(Qt::UserRole).toLongLong();
      const auto addr2 = other.data(Qt::UserRole).toLongLong();

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
  selectBlock(blockNum);
}

void BinaryWidget::selectBlock(int number)
{
  auto block = doc->findBlockByNumber(number);
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

void BinaryWidget::checkModified(const Section *section,
                                 const QList<Section::ModifiedRegion> &priorModifications)
{
  // Only emit modified if new changes were made.
  if (section->isModified() && section->modifiedRegions() != priorModifications) {
    emit modified();

    auto ret = QMessageBox::question(this, "dispar", tr("Binary was modified. Reload UI?"),
                                     QMessageBox::Yes | QMessageBox::No);
    if (QMessageBox::Yes == ret) {
      reloadUi();
    }
  }
}

} // namespace dispar
