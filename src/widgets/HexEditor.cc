#include "widgets/HexEditor.h"
#include "Util.h"
#include "widgets/TreeWidget.h"

#include <QApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QHash>
#include <QLabel>
#include <QLineEdit>
#include <QProgressDialog>
#include <QStyledItemDelegate>
#include <QTreeWidget>
#include <QVBoxLayout>

namespace {

class ItemDelegate : public QStyledItemDelegate {
public:
  ItemDelegate(QTreeWidget *tree, Section *section) : tree(tree), section(section)
  {
  }

  QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                        const QModelIndex &index) const
  {
    const auto col = index.column();
    if (col != 1 && col != 2) {
      return nullptr;
    }

    const auto text = index.data().toString().trimmed();
    if (text.isEmpty()) {
      return nullptr;
    }

    QString mask;
    int blocks = text.split(" ").size();
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

    const auto col = index.column();
    Util::setTreeItemMarked(item, col);

    // Generate new ASCII representation.
    const auto oldAscii = item->text(3);
    auto newAscii = Util::hexToAscii(newStr, 0, 8);
    if (col == 1) {
      newAscii += oldAscii.mid(8);
    }
    else {
      newAscii = oldAscii.mid(0, 8) + newAscii;
    }
    item->setText(3, newAscii);

    // Change region.
    quint64 addr = item->text(0).toULongLong(nullptr, 16);
    quint64 pos = (addr - section->address()) + (col - 1) * 8;
    QByteArray data = Util::hexToData(newStr.replace(" ", ""));
    section->setSubData(data, pos);
  }

private:
  QTreeWidget *tree;
  Section *section;
};

} // namespace

HexEditor::HexEditor(Section *section, BinaryObject *object, QWidget *parent)
  : QDialog(parent), section(section), object(object), shown(false), rows(0)
{
  Q_ASSERT(section);
  Q_ASSERT(object);

  setWindowTitle(tr("Hex Editor: %1").arg(section->toString()));
  createLayout();
}

void HexEditor::showEvent(QShowEvent *event)
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

void HexEditor::createLayout()
{
  label = new QLabel;

  treeWidget = new TreeWidget;
  treeWidget->setHeaderLabels({tr("Address"), tr("Data Low"), tr("Data High"), tr("ASCII")});
  treeWidget->setColumnWidth(0, object->systemBits() == 64 ? 110 : 70);
  treeWidget->setColumnWidth(1, 200);
  treeWidget->setColumnWidth(2, 200);
  treeWidget->setColumnWidth(3, 110);
  treeWidget->setItemDelegate(new ItemDelegate(treeWidget, section));
  treeWidget->setMachineCodeColumns(QList<int>{1, 2});
  treeWidget->setCpuType(object->cpuType());
  treeWidget->setAddressColumn(0);

  auto *layout = new QVBoxLayout;
  layout->setContentsMargins(5, 5, 5, 5);
  layout->addWidget(label);
  layout->addWidget(treeWidget);

  setLayout(layout);
}

void HexEditor::setup()
{
  createEntries();
  markModifiedRegions();

  int padSize = object->systemBits() / 8;
  const auto addr = section->address();
  const auto len = section->data().size();
  label->setText(tr("Section size: %1, address %2 to %3, %4 rows")
                   .arg(Util::formatSize(len))
                   .arg(Util::padString(QString::number(addr, 16).toUpper(), padSize))
                   .arg(Util::padString(QString::number(addr + len, 16).toUpper(), padSize))
                   .arg(treeWidget->topLevelItemCount()));

  treeWidget->setFocus();
}

void HexEditor::createEntries()
{
  QElapsedTimer elapsedTimer;
  elapsedTimer.start();

  qDebug() << "Generating UI for hex editor..";
  treeWidget->clear();

  quint64 addr = section->address();
  const QByteArray &data = section->data();
  int len = data.size();
  rows = len / 16;

  if (len == 0) {
    label->setText(tr("Defined but empty."));
    treeWidget->hide();
    return;
  }

  if (len % 16 > 0) rows++;

  QHash<unsigned char, QString> charToHex;
  for (int row = 0, byte = 0; row < rows; row++, addr += 16) {
    auto *item = new QTreeWidgetItem;
    item->setFlags(Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    item->setText(0,
                  Util::padString(QString::number(addr, 16).toUpper(), object->systemBits() / 8));

    QStringList codes;
    int start = byte, end = 0;
    for (; end < 16 && byte < len; end++, byte++) {
      const auto ch = static_cast<unsigned char>(data[byte]);

      // Use map to speed up the process because there will be a max of 256 values!
      auto hex = charToHex.value(ch);
      if (hex.isEmpty()) {
        hex = Util::padString(QString::number(ch, 16), 2);
        charToHex[ch] = hex;
      }

      codes << hex;
    }

    const auto code = codes.join(" ").toUpper();
    item->setText(1, code.mid(0, 8 * 3));
    item->setText(2, code.mid(8 * 3));

    const auto ascii = Util::dataToAscii(data, start, end);
    item->setText(3, ascii);

    treeWidget->addTopLevelItem(item);
  }

  qDebug() << ">" << elapsedTimer.restart() << "ms";
}

void HexEditor::markModifiedRegions()
{
  const auto &modRegs = section->modifiedRegions();
  if (modRegs.isEmpty()) return;

  QElapsedTimer elapsedTimer;
  elapsedTimer.start();

  qDebug() << "Marking modified regions in UI..";

  for (int row = 0, byte = 0; row < rows; row++, byte += 16) {
    auto *item = treeWidget->topLevelItem(row);
    for (const auto &reg : modRegs) {
      if (reg.first >= byte && reg.first < byte + 16) {
        int col1 = 1, col2 = 2;
        if (reg.first < byte + 8) {
          Util::setTreeItemMarked(item, col1);
        }

        if (reg.first + reg.second >= byte + 8) {
          Util::setTreeItemMarked(item, col2);
        }

        if (reg.first + reg.second > byte + 16) {
          // Number of additional rows to mark.
          int num = ((reg.first + reg.second) - (byte + 16)) / 16;
          for (int j = 0; j < num + 1; j++) {
            item = treeWidget->topLevelItem(row + j + 1);
            if (!item) continue;

            Util::setTreeItemMarked(item, col1);

            // If intermediate rows or if the data actually spans
            // the last column.
            if (j < num || (reg.first + reg.second) % 16 > 8) {
              Util::setTreeItemMarked(item, col2);
            }
          }
        }
      }
    }
  }

  qDebug() << ">" << elapsedTimer.restart() << "ms";
}
