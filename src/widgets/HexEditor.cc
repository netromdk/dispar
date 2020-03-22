#include "widgets/HexEditor.h"
#include "Constants.h"
#include "Context.h"
#include "Util.h"
#include "widgets/HexEdit.h"

#include <cassert>

#include <QApplication>
#include <QDebug>
#include <QElapsedTimer>
#include <QHash>
#include <QLabel>
#include <QLineEdit>
#include <QProgressDialog>
#include <QStyledItemDelegate>
#include <QTimer>
#include <QVBoxLayout>

namespace dispar {

HexEditor::HexEditor(Section *section_, BinaryObject *object_, QWidget *parent)
  : QDialog(parent), section(section_), object(object_), shown(false)
{
  assert(section);
  assert(object);

  setWindowTitle(tr("Hex Editor: %1").arg(section->toString()));
  createLayout();
}

HexEditor::~HexEditor()
{
  Context::get().setValue("HexEditor.geometry", Util::byteArrayString(saveGeometry()));
}

void HexEditor::showEvent(QShowEvent *event)
{
  QDialog::showEvent(event);

  if (!shown) {
    shown = true;

    // Setup after showing UI.
    QTimer::singleShot(1, this, &HexEditor::setup);

    if (!restoreGeometry(Util::byteArray(Context::get().value("HexEditor.geometry").toString()))) {
      resize(800, 600);
      Util::centerWidget(this);
    }
  }
  else if (section->isModified()) {
    const auto mod = section->modifiedWhen();
    if (sectionModified.isNull() || mod != sectionModified) {
      sectionModified = mod;
      setup();
    }
  }
}

void HexEditor::createLayout()
{
  label = new QLabel;

  textEdit = new HexEdit(object->cpuType());
  connect(textEdit, &HexEdit::edited, this, &HexEditor::updateModified);

  auto *layout = new QVBoxLayout;
  layout->setContentsMargins(5, 5, 5, 5);
  layout->addWidget(label);
  layout->addWidget(textEdit);

  setLayout(layout);
}

void HexEditor::setup()
{
  progDiag = new QProgressDialog(this);
  progDiag->setWindowFlags(progDiag->windowFlags() | Qt::WindowStaysOnTopHint);
  progDiag->setLabelText(tr("Loading entries.."));
  progDiag->setCancelButton(nullptr);
  progDiag->setRange(0, 0);
  progDiag->show();
  qApp->processEvents();

  auto start = QDateTime::currentMSecsSinceEpoch();

  createEntries();

  int padSize = object->systemBits() / 8;
  const auto addr = section->address();
  const auto len = section->data().size();
  label->setText(tr("Section size: %1, address %2 to %3")
                   .arg(Util::formatSize(len))
                   .arg(Util::padString(QString::number(addr, 16).toUpper(), padSize))
                   .arg(Util::padString(QString::number(addr + len, 16).toUpper(), padSize)));

  // Remove dialog when all events related to the adding and drawing of text edit items have been
  // processed. This is when this additional event is processed.
  QTimer::singleShot(1, this, [this, start] {
    auto end = QDateTime::currentMSecsSinceEpoch();
    qDebug() << ">" << (end - start) << "ms";
    delete progDiag;
    progDiag = nullptr;
    qDebug() << "done with entries";
  });
}

void HexEditor::createEntries()
{
  QElapsedTimer elapsedTimer;
  elapsedTimer.start();

  qDebug() << "Generating UI for hex editor..";

  const auto &sectionData = section->data();
  int len = sectionData.size();

  if (len == 0) {
    label->setText(tr("Defined but empty."));
    textEdit->hide();
    return;
  }

  if (progDiag) {
    progDiag->setLabelText(tr("Loading %1 data)").arg(Util::formatSize(len)));
    qApp->processEvents();
  }

  textEdit->setFocus();
  textEdit->decode(section);

  qDebug() << ">" << elapsedTimer.restart() << "ms";
}

// *************************************************************************************************
void HexEditor::updateModified()
// *************************************************************************************************
{
  sectionModified = QDateTime::currentDateTime();
}

} // namespace dispar
