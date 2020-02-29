#include "widgets/MacSdkVersionsEditor.h"
#include "Context.h"
#include "MacSdkVersionPatcher.h"
#include "Reader.h"
#include "Util.h"

#include <cassert>

#include <QBuffer>
#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>

namespace dispar {

MacSdkVersionsEditor::MacSdkVersionsEditor(Section *section, BinaryObject *object, QWidget *parent)
  : QDialog(parent), section(section), patcher(*section)
{
  assert(section);
  Q_UNUSED(object);

  setWindowTitle(tr("Versions Editor: %1").arg(section->toString()));
  createLayout();
}

MacSdkVersionsEditor::~MacSdkVersionsEditor()
{
  Context::get().setValue("MacSdkVersionsEditor.geometry", Util::byteArrayString(saveGeometry()));
}

void MacSdkVersionsEditor::showEvent(QShowEvent *event)
{
  QDialog::showEvent(event);

  if (!shown) {
    shown = true;
    setup();

    if (!restoreGeometry(
          Util::byteArray(Context::get().value("MacSdkVersionsEditor.geometry").toString()))) {
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

void MacSdkVersionsEditor::setup()
{
  readValues();
}

void MacSdkVersionsEditor::accept()
{
  const MacSdkVersionPatcher::Version newTarget(targetMajorSpin->value(), targetMinorSpin->value()),
    newSdk(sdkMajorSpin->value(), sdkMinorSpin->value());

  // OR to call both setter functions and yield true if one of them were true. If || is used then
  // only the first setter is called if it returns true.
  if (patcher.setTarget(newTarget) | patcher.setSdk(newSdk)) {
    sectionModified = QDateTime::currentDateTime();
    done(QDialog::Accepted);
    return;
  }

  // Not modified.
  done(QDialog::Rejected);
}

void MacSdkVersionsEditor::readValues()
{
  if (!patcher.valid()) return;

  const auto target = patcher.target();
  targetMajorSpin->setValue(std::get<0>(target));
  targetMinorSpin->setValue(std::get<1>(target));

  const auto sdk = patcher.sdk();
  sdkMajorSpin->setValue(std::get<0>(sdk));
  sdkMinorSpin->setValue(std::get<1>(sdk));
}

void MacSdkVersionsEditor::createLayout()
{
  targetMajorSpin = new QSpinBox;
  targetMajorSpin->setRange(1, 100);

  targetMinorSpin = new QSpinBox;
  targetMinorSpin->setRange(1, 100);

  auto *targetLayout = new QHBoxLayout;
  targetLayout->addWidget(new QLabel(tr("Target SDK version:")));
  targetLayout->addWidget(targetMajorSpin);
  targetLayout->addWidget(targetMinorSpin);

  sdkMajorSpin = new QSpinBox;
  sdkMajorSpin->setRange(1, 100);

  sdkMinorSpin = new QSpinBox;
  sdkMinorSpin->setRange(1, 100);

  auto *sdkLayout = new QHBoxLayout;
  sdkLayout->addWidget(new QLabel(tr("SDK version:")));
  sdkLayout->addWidget(sdkMajorSpin);
  sdkLayout->addWidget(sdkMinorSpin);

  auto *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
  connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

  auto *layout = new QVBoxLayout;
  layout->setContentsMargins(5, 5, 5, 5);
  layout->addLayout(targetLayout);
  layout->addLayout(sdkLayout);
  layout->addWidget(buttonBox);

  setLayout(layout);
}

} // namespace dispar
