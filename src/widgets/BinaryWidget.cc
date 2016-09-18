#include <QTimer>
#include <QFile>
#include <QTextEdit>
#include <QDebug>
#include <QListWidget>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QApplication>
#include <QProgressDialog>
#include <QTextTable>

#include "../Util.h"
#include "BinaryWidget.h"

#include <X86Disasm.hh>
#include <Disasm.hpp>

namespace {

// Temporary solution!
class TextBlockUserData : public QTextBlockUserData {
public:
  QString toString()
  {
    if (addr) {
      return "address";
    }
    else if (inst) {
      return "instruction";
    }
    else {
      return "operands";
    }
  }

  bool addr = false, inst = false, ops = false;
};

} // anon

BinaryWidget::BinaryWidget(std::shared_ptr<Format> fmt) : fmt(fmt), doc(nullptr)
{
  createLayout();
  setup();
}

QString BinaryWidget::file() const
{
  return fmt->file();
}

void BinaryWidget::onSymbolChosen(int row)
{
  // If offset is found then select the text block.
  auto *item = symbolList->item(row);
  auto offset = item->data(Qt::UserRole).toLongLong();
  if (offsetBlock.contains(offset)) {
    auto blockNum = offsetBlock[offset];
    auto block = doc->findBlockByNumber(blockNum);
    auto cursor = mainView->textCursor();
    cursor.setPosition(block.position());
    cursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
    mainView->setTextCursor(cursor);
    mainView->ensureCursorVisible();
    mainView->setFocus();
  }
}

void BinaryWidget::onCursorPositionChanged()
{
  auto cursor = mainView->textCursor();
  auto block = cursor.block();
  qDebug() << "column:" << cursor.columnNumber() << "block:" << block.blockNumber();

  auto *userData = reinterpret_cast<TextBlockUserData *>(block.userData());
  if (userData) {
    qDebug() << userData->toString();
  }
}

void BinaryWidget::createLayout()
{
  symbolList = new QListWidget;
  symbolList->setFixedWidth(175);
  connect(symbolList, &QListWidget::currentRowChanged, this, &BinaryWidget::onSymbolChosen);

  mainView = new QTextEdit;
  mainView->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
  connect(mainView, &QTextEdit::cursorPositionChanged, this,
          &BinaryWidget::onCursorPositionChanged);

  doc = mainView->document();

  auto *layout = new QHBoxLayout;
  layout->setContentsMargins(5, 5, 5, 5);
  layout->addWidget(symbolList);
  layout->addWidget(mainView);

  setLayout(layout);
}

void BinaryWidget::setup()
{
  // For now we just support one object!

  auto obj = fmt->objects()[0];

  // Fill side bar with function names of the symbol table.
  for (auto symbol : obj->symbolTable().symbols()) {
    auto *item = new QListWidgetItem;
    item->setText(symbol.string());
    item->setData(Qt::UserRole, symbol.value()); // Offset to symbol.
    item->setToolTip(QString("0x%1").arg(symbol.value(), 0, 16));
    symbolList->addItem(item);
  }

  // Disassemble test! ============
  CX86Disasm64 dis;

  // check if no error occured
  if (dis.GetError()) return;

  // set how deep should capstone reverse instruction
  dis.SetDetail(cs_opt_value::CS_OPT_ON);

  // set syntax for output disasembly string
  dis.SetSyntax(cs_opt_value::CS_OPT_SYNTAX_INTEL);

  // process disasembling
  auto textSec = obj->section(Section::Type::TEXT);
  auto *code = textSec->data().constData();
  auto *insn = dis.Disasm(code, textSec->size());

  // check if disassembling succesfull
  if (!insn) {
    qFatal("disasm failed!");
  }

  // print basic info
  for (size_t i = 0; i < insn->Count; i++) {
    printf("-> 0x%llu:\t%s\t%s\n", insn->Instructions(i)->address, insn->Instructions(i)->mnemonic,
           insn->Instructions(i)->op_str);
  }

  // =========================

  // Create text edit of all binary contents.
  // TODO: FAKE IT FOW NOW!
  QTextCursor cursor(doc);

  auto createTable = [this, &cursor](const QStringList &values) {
    cursor.movePosition(QTextCursor::End);
    auto *table = cursor.insertTable(1, 3);
    Q_ASSERT(values.size() <= 3);

    auto format = table->format();
    QVector<QTextLength> colWidths;
    colWidths << QTextLength(QTextLength::FixedLength, 200)
              << QTextLength(QTextLength::FixedLength, 40)
              << QTextLength(QTextLength::VariableLength, 1);
    format.setColumnWidthConstraints(colWidths);
    format.setBorder(0);
    format.setBorderStyle(QTextFrameFormat::BorderStyle_None);
    table->setFormat(format);

    int col = 0;
    for (const auto &value : values) {
      int cellCol = col++;
      auto cellCursor = table->cellAt(0, cellCol).firstCursorPosition();
      cellCursor.insertText(value);

      auto *userData = new TextBlockUserData;
      userData->addr = (cellCol == 0);
      userData->inst = (cellCol == 1);
      userData->ops = (cellCol == 2);

      auto block = cellCursor.block();
      block.setUserData(userData);

      if (cellCol == 0) {
        offsetBlock[value.toLongLong(nullptr, 16)] = block.blockNumber();
      }
    }
  };

  cursor.beginEditBlock();

  cursor.movePosition(QTextCursor::End);

  // There is a default block at the beginning so reuse that.
  if (cursor.block() != doc->firstBlock()) {
    cursor.insertBlock();
  }
  cursor.insertText("_main:");

  for (size_t i = 0; i < insn->Count; i++) {
    auto instr = insn->Instructions(i);
    createTable(QStringList{QString("0x%1").arg(instr->address + textSec->address(), 0, 16),
                            instr->mnemonic, instr->op_str});
  }

  cursor.movePosition(QTextCursor::End);
  cursor.insertBlock();
  cursor.insertText("; endp");

  cursor.endEditBlock();

  Util::scrollToTop(mainView);

  // The "_main" symbol is normally the second symbol so choose that. If not enough symbols then
  // choose the first.
  QTimer::singleShot(1, this, [this] {
    auto count = symbolList->count();
    if (count > 1) {
      symbolList->setCurrentRow(1);
    }
    else if (count > 0) {
      symbolList->setCurrentRow(0);
    }
  });
}
