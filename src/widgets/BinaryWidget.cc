#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProgressDialog>
#include <QTabWidget>
#include <QTextBlockUserData>
#include <QTimer>

#include "../BinaryObject.h"
#include "../CStringReader.h"
#include "../Context.h"
#include "../Util.h"
#include "BinaryWidget.h"
#include "PersistentSplitter.h"

namespace {

// Temporary solution!
class TextBlockUserData : public QTextBlockUserData {
public:
  quint64 address;
  int addressStart, addressEnd;
  int bytesStart, bytesEnd;
  QString bytes;
};

} // anon

BinaryWidget::BinaryWidget(std::shared_ptr<BinaryObject> &object)
  : object(object), shown(false), doc(nullptr)
{
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

void BinaryWidget::onSymbolChosen(int row)
{
  auto *list = qobject_cast<QListWidget *>(sender());

  // If offset is found then select the text block.
  auto *item = list->item(row);
  auto offset = item->data(Qt::UserRole).toLongLong();
  if (offsetBlock.contains(offset)) {
    auto blockNum = offsetBlock[offset];
    auto block = doc->findBlockByNumber(blockNum);
    auto cursor = mainView->textCursor();
    cursor.setPosition(block.position());
    cursor.movePosition(QTextCursor::EndOfWord, QTextCursor::KeepAnchor);
    mainView->setTextCursor(cursor);
    mainView->ensureCursorVisible();
  }
}

void BinaryWidget::onCursorPositionChanged()
{
  auto cursor = mainView->textCursor();
  auto block = cursor.block();
  qDebug() << "column:" << cursor.columnNumber() << "block:" << block.blockNumber();

  auto *userData = dynamic_cast<TextBlockUserData *>(block.userData());
  if (userData) {
    qDebug() << "address:" << QString::number(userData->address, 16);
  }

  // auto text = block.text();
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

  auto start = QDateTime::currentDateTime();

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

  auto end = QDateTime::currentDateTime();
  qDebug() << "Modified machine code visibility in" << start.msecsTo(end) << "ms";
}

void BinaryWidget::filterSymbols(const QString &filter)
{
  auto *list = (symbolList->isVisible() ? symbolList : stringList);

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
  symbolList = new QListWidget;
  connect(symbolList, &QListWidget::currentRowChanged, this, &BinaryWidget::onSymbolChosen);

  stringList = new QListWidget;
  connect(stringList, &QListWidget::currentRowChanged, this, &BinaryWidget::onSymbolChosen);

  auto *tabWidget = new QTabWidget;
  tabWidget->addTab(symbolList, tr("Functions"));
  tabWidget->addTab(stringList, tr("Strings"));

  auto *filterSymLine = new QLineEdit;
  filterSymLine->setPlaceholderText(tr("Filter symbols.."));
  connect(filterSymLine, &QLineEdit::textEdited, this, &BinaryWidget::filterSymbols);

  auto *symbolsLayout = new QVBoxLayout;
  symbolsLayout->setContentsMargins(0, 0, 0, 0);
  symbolsLayout->addWidget(tabWidget);
  symbolsLayout->addWidget(filterSymLine);

  auto *symbolsWidget = new QWidget;
  symbolsWidget->setLayout(symbolsLayout);

  mainView = new QPlainTextEdit;
  mainView->setReadOnly(true);
  mainView->setCenterOnScroll(true);
  mainView->setLineWrapMode(QPlainTextEdit::NoWrap);
  mainView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
  connect(mainView, &QPlainTextEdit::cursorPositionChanged, this,
          &BinaryWidget::onCursorPositionChanged);

  doc = mainView->document();

  auto *vertSplitter = new PersistentSplitter("BinaryWidget.vertSplitter");
  vertSplitter->addWidget(symbolsWidget);
  vertSplitter->addWidget(mainView);

  vertSplitter->setSizes(QList<int>{175, 500});

  auto *layout = new QHBoxLayout;
  layout->setContentsMargins(5, 5, 5, 5);
  layout->addWidget(vertSplitter);

  setLayout(layout);
}

void BinaryWidget::setup()
{
  auto start = QDateTime::currentDateTime();

  QProgressDialog setupDiag(this);
  setupDiag.setLabelText(tr("Setting up for binary data.."));
  setupDiag.setCancelButton(nullptr);
  setupDiag.setRange(0, 4);
  setupDiag.show();
  qApp->processEvents();
  qDebug() << qPrintable(setupDiag.labelText());

  auto symbols = object->symbolTable().symbols();
  symbols.append(object->dynSymbolTable().symbols());

  // Create text edit of all binary contents.
  QTextCursor cursor(doc);

  auto &ctx = Context::get();

  auto appendInstruction = [this, &cursor, &ctx](const QStringList &values) {
    Q_ASSERT(values.size() == 4);

    auto *userData = new TextBlockUserData;
    userData->address = values[0].toLongLong(nullptr, 16);
    userData->addressStart = 0;
    userData->addressEnd = 20;

    bool smc = ctx.showMachineCode();
    userData->bytes = values[1];
    userData->bytesStart = userData->addressEnd + 1;
    userData->bytesEnd = (smc ? userData->bytesStart + 24 : -1);

    cursor.insertBlock();
    cursor.insertText(QString("%1%2%3%4")
                        .arg(values[0], -20)
                        .arg(smc ? QString("%1").arg(values[1], -24) : QString())
                        .arg(values[2], -10)
                        .arg(values[3]));

    auto block = cursor.block();
    block.setUserData(userData);

    offsetBlock[userData->address] = block.blockNumber();
    codeBlocks << block.blockNumber();
  };

  auto appendString = [this, &cursor](const quint64 &address, const QString &string) {
    cursor.insertBlock();
    cursor.insertText(QString("%1%2%3")
                        .arg(QString("0x%1").arg(address, 0, 16), -20)
                        .arg(QString("\"%1\"").arg(string))
                        .arg(tr("; size=%1").arg(string.size()), 11));

    auto *userData = new TextBlockUserData;
    userData->address = address;

    auto block = cursor.block();
    block.setUserData(userData);

    offsetBlock[userData->address] = block.blockNumber();
  };

  cursor.beginEditBlock();

  setupDiag.setValue(1);
  setupDiag.setLabelText(tr("Generating UI for disassembled sections.."));
  qApp->processEvents();
  qDebug() << qPrintable(setupDiag.labelText());

  for (auto &sec : object->sections()) {
    auto disasm = sec->disassembly();
    if (!disasm) continue;

    cursor.movePosition(QTextCursor::End);

    // There is a default block at the beginning so reuse that.
    if (cursor.block() != doc->firstBlock()) {
      cursor.insertBlock();
    }

    auto secName = Section::typeName(sec->type());
    cursor.insertText("===== " + secName + " =====");

    for (size_t i = 0; i < disasm->count(); i++) {
      auto *instr = disasm->instructions(i);
      auto addr = instr->address + sec->address();

      // Check if address is the start of a procedure.
      for (const auto &symbol : symbols) {
        if (symbol.value() == addr && !symbol.string().isEmpty()) {
          cursor.movePosition(QTextCursor::End);
          cursor.insertBlock();
          cursor.insertText("\nPROC: " + Util::demangle(symbol.string()) + "\n");
          break;
        }
      }

      appendInstruction(QStringList{QString("0x%1").arg(addr, 0, 16),
                                    Util::bytesToHex(instr->bytes, instr->size), instr->mnemonic,
                                    instr->op_str});
    }

    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock();
    cursor.insertText("\n===== /" + secName + " =====\n");
  }

  setupDiag.setValue(2);
  setupDiag.setLabelText(tr("Generating UI for string sections.."));
  qApp->processEvents();
  qDebug() << qPrintable(setupDiag.labelText());

  // Show cstring+string sections.
  auto stringSecs = object->sectionsByType(Section::Type::CSTRING);
  stringSecs << object->sectionsByType(Section::Type::STRING);
  for (auto &sec : stringSecs) {
    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock();
    auto secName = Section::typeName(sec->type());
    cursor.insertText("===== " + secName + " =====\n");

    CStringReader reader(sec->data());
    while (reader.next()) {
      auto addr = reader.offset() + sec->address();
      auto string = reader.string();

      appendString(addr, string);

      auto *item = new QListWidgetItem;
      item->setText(reader.string());
      item->setData(Qt::UserRole, addr);
      item->setToolTip(QString("0x%1").arg(addr, 0, 16));
      stringList->addItem(item);
    }

    cursor.movePosition(QTextCursor::End);
    cursor.insertBlock();
    cursor.insertText("\n===== /" + secName + " =====\n");
  }

  setupDiag.setValue(3);
  setupDiag.setLabelText(tr("Generating sidebar with functions and strings.."));
  qApp->processEvents();
  qDebug() << qPrintable(setupDiag.labelText());

  // Fill side bar with function names of the symbol tables.
  for (auto symbol : symbols) {
    if (symbol.value() == 0) continue;
    auto *item = new QListWidgetItem;

    auto func = Util::demangle(symbol.string());
    if (func.isEmpty()) {
      func = QString("unnamed_%1").arg(symbol.value(), 0, 16);
    }
    if (offsetBlock.contains(symbol.value())) {
      func += " *";
    }
    item->setText(func);

    item->setData(Qt::UserRole, symbol.value()); // Offset to symbol.
    item->setToolTip(QString("0x%1").arg(symbol.value(), 0, 16));
    symbolList->addItem(item);
  }

  cursor.endEditBlock();
  setupDiag.setValue(4);

  Util::scrollToTop(mainView);

  auto end = QDateTime::currentDateTime();
  qDebug() << "Setup in" << start.msecsTo(end) << "ms";
}
