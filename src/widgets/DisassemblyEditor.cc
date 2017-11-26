#include "widgets/DisassemblyEditor.h"
#include "BinaryObject.h"
#include "Context.h"
#include "Section.h"
#include "Util.h"
#include "widgets/TreeWidget.h"

#include <memory>

#include <QDebug>
#include <QElapsedTimer>
#include <QHBoxLayout>
#include <QHash>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QStyledItemDelegate>
#include <QVBoxLayout>

namespace dispar {

namespace {

class ItemDelegate : public QStyledItemDelegate {
public:
  ItemDelegate(DisassemblyEditor *disasmEditor, QTreeWidget *tree, BinaryObject *object,
               Section *section)
    : disasmEditor(disasmEditor), tree(tree), object(object), section(section)
  {
  }

  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                        const QModelIndex &index) const
  {
    const auto col = index.column();
    if (col != 1) {
      return nullptr;
    }

    const auto text = index.data().toString().trimmed();
    if (text.isEmpty()) {
      return nullptr;
    }

    QString mask;
    const auto blocks = text.split(" ").size();
    for (int i = 0; i < blocks; i++) {
      mask += "HH ";
    }
    if (mask.endsWith(" ")) {
      mask.chop(1);
    }

    auto *edit = new QLineEdit(parent);
    edit->setInputMask(mask);
    edit->setText(text);
    return edit;
  }

  void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
  {
    auto *edit = qobject_cast<QLineEdit *>(editor);
    if (!edit) return;

    const auto oldStr = model->data(index).toString().trimmed();
    auto newStr = edit->text().toUpper();
    if (newStr == oldStr) {
      return;
    }

    model->setData(index, newStr);
    auto *item = tree->topLevelItem(index.row());
    if (!item) return;
    Util::setTreeItemMarked(item, index.column());

    // Change region.
    const auto addr = item->text(0).toULongLong(nullptr, 16);
    const auto pos = addr - section->address();
    const auto data = Util::hexToData(newStr.replace(" ", ""));
    section->setSubData(data, pos);

    // Update disassembly.
    Disassembler dis(*object, Context::get().disassemblerSyntax());
    const auto result = dis.disassemble(data, addr);
    if (!result) {
      item->setText(2, tr("Could not disassemble!"));
    }
    else {
      QStringList lines;
      for (std::size_t i = 0; i < result->count(); i++) {
        const auto *instr = result->instructions(i);
        lines << QString("%1 %2").arg(instr->mnemonic).arg(instr->op_str);
      }
      item->setText(2, lines.join("   "));

      if (result->count() > 1) {
        disasmEditor->showUpdateButton();
        QMessageBox::information(nullptr, "",
                                 tr("Changes implied new instructions.") + "\n" +
                                   tr("Disassemble again for clear representation."));
      }
    }
  }

private:
  DisassemblyEditor *disasmEditor;
  QTreeWidget *tree;
  BinaryObject *object;
  Section *section;
};

} // namespace

DisassemblyEditor::DisassemblyEditor(Section *section, BinaryObject *object, QWidget *parent)
  : QDialog(parent), section(section), object(object), shown(false), label(nullptr),
    treeWidget(nullptr)
{
  Q_ASSERT(section);
  Q_ASSERT(section->disassembly());
  Q_ASSERT(object);

  setWindowTitle(tr("Disassembly Editor: %1").arg(section->toString()));
  createLayout();
}

void DisassemblyEditor::showUpdateButton()
{
  updateButton->show();
}

void DisassemblyEditor::done(int result)
{
  // Update disassembly if changed before closing dialog.
  if (section->isModified()) {
    updateDisassembly();
  }

  QDialog::done(result);
}

void DisassemblyEditor::showEvent(QShowEvent *event)
{
  QDialog::showEvent(event);

  if (!shown) {
    shown = true;
    setup();
  }

  Util::delayFunc([this] {
    resize(800, 600);
    Util::centerWidget(this);
  });
}

void DisassemblyEditor::updateDisassembly()
{
  QElapsedTimer elapsedTimer;
  elapsedTimer.start();

  qDebug() << "Re-dissassembling..";

  Disassembler dis(*object, Context::get().disassemblerSyntax());
  auto result = dis.disassemble(section->data());
  if (!result) {
    QMessageBox::critical(this, "", tr("Could not disassemble machine code!"));
    return;
  }

  section->setDisassembly(std::move(result));
  qDebug() << ">" << elapsedTimer.restart() << "ms";
}

void DisassemblyEditor::createLayout()
{
  label = new QLabel;

  updateButton = new QPushButton(tr("Update disassembly"));
  updateButton->hide();
  connect(updateButton, &QPushButton::clicked, this, [this] {
    updateDisassembly();
    setup();
  });

  auto *topLayout = new QHBoxLayout;
  topLayout->setContentsMargins(5, 5, 5, 5);
  topLayout->addWidget(label);
  topLayout->addStretch();
  topLayout->addWidget(updateButton);

  treeWidget = new TreeWidget;
  treeWidget->setHeaderLabels(QStringList{tr("Address"), tr("Data"), tr("Disassembly")});
  treeWidget->setColumnWidth(0, object->systemBits() == 64 ? 110 : 70);
  treeWidget->setColumnWidth(1, 200);
  treeWidget->setColumnWidth(2, 200);
  treeWidget->setItemDelegate(new ItemDelegate(this, treeWidget, object, section));
  treeWidget->setMachineCodeColumns(QList<int>{1});
  treeWidget->setCpuType(object->cpuType());
  treeWidget->setAddressColumn(0);

  auto *layout = new QVBoxLayout;
  layout->setContentsMargins(5, 5, 5, 5);
  layout->addLayout(topLayout);
  layout->addWidget(treeWidget);

  setLayout(layout);
}

void DisassemblyEditor::setup()
{
  createEntries();
  markModifiedRegions();
  treeWidget->setFocus();
}

void DisassemblyEditor::createEntries()
{
  QElapsedTimer elapsedTimer;
  elapsedTimer.start();

  qDebug() << "Generating UI for disassembly editor..";
  treeWidget->clear();

  auto *disasm = section->disassembly();
  label->setText(tr("%1 instructions").arg(disasm->count()));

  const auto &symbols = object->symbolTable().symbols();

  // Create temporary procedure name lookup map.
  QHash<quint64, QString> procNameMap;
  for (const auto &symbol : symbols) {
    if (!symbol.string().isEmpty()) {
      procNameMap[symbol.value()] = Util::demangle(symbol.string());
    }
  }

  for (std::size_t i = 0; i < disasm->count(); i++) {
    const auto *instr = disasm->instructions(i);
    const auto offset = instr->address;
    const auto addr = offset + section->address();

    // Check if address is the start of a procedure.
    const auto it = procNameMap.find(addr);
    if (it != procNameMap.end()) {
      auto *item = new QTreeWidgetItem;
      item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

      const int col = 2;
      item->setText(col, *it);

      auto font = item->font(col);
      font.setBold(true);
      item->setFont(col, font);

      if (i > 0) {
        treeWidget->addTopLevelItem(new QTreeWidgetItem);
      }
      treeWidget->addTopLevelItem(item);
    }

    auto *item = new QTreeWidgetItem;
    item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    item->setText(0,
                  Util::padString(QString::number(addr, 16).toUpper(), object->systemBits() / 8));
    item->setText(1, Util::bytesToHex(instr->bytes, instr->size));
    item->setText(2, QString("%1 %2").arg(instr->mnemonic).arg(instr->op_str));
    treeWidget->addTopLevelItem(item);
  }

  qDebug() << ">" << elapsedTimer.restart() << "ms";
}

void DisassemblyEditor::markModifiedRegions()
{
  const auto &modRegs = section->modifiedRegions();
  if (modRegs.isEmpty()) return;

  QElapsedTimer elapsedTimer;
  elapsedTimer.start();

  qDebug() << "Marking modified regions in UI..";

  int rows = treeWidget->topLevelItemCount();
  quint64 offset = section->address();
  quint64 addr = 0;
  for (int row = 0; row < rows; row++) {
    auto *item = treeWidget->topLevelItem(row);

    // Skip procedure starts.
    if (item->text(0).isEmpty()) {
      continue;
    }

    addr = item->text(0).toULongLong(nullptr, 16) - offset;
    int size = item->text(1).split(" ", QString::SkipEmptyParts).size();
    for (const auto &reg : modRegs) {
      const auto first = static_cast<quint64>(reg.first);
      if (first >= addr && first < addr + size) {
        Util::setTreeItemMarked(item, 1);
        auto excess = static_cast<quint64>(reg.first + reg.second) - (addr + size);
        if (excess == 0) continue;

        for (int row2 = row + 1; row2 < rows; row2++) {
          auto *item2 = treeWidget->topLevelItem(row2);
          if (item2) {
            int size2 = item2->text(1).split(" ", QString::SkipEmptyParts).size();
            Util::setTreeItemMarked(item2, 1);
            excess -= size2;
            if (excess <= 0) break;
          }
          else
            break;
        }
      }
    }
  }

  qDebug() << ">" << elapsedTimer.restart() << "ms";
}

} // namespace dispar
