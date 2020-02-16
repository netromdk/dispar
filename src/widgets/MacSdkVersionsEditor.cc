#include "widgets/MacSdkVersionsEditor.h"
#include "Context.h"
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
  : QDialog(parent), section(section)
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
  const auto applyVersion = [&](quint64 addr, int major, int minor, int curMajor, int curMinor) {
    // Don't update if values are the same.
    if (major == curMajor && minor == curMinor) {
      return;
    }

    const auto version = Util::encodeMacSdkVersion({major, minor});
    section->setSubData(Util::longToData(version), addr);

    sectionModified = QDateTime::currentDateTime();
  };

  applyVersion(targetAddr, targetMajorSpin->value(), targetMinorSpin->value(), targetMajor,
               targetMinor);
  applyVersion(sdkAddr, sdkMajorSpin->value(), sdkMinorSpin->value(), sdkMajor, sdkMinor);

  done(QDialog::Accepted);
}

void MacSdkVersionsEditor::readValues()
{
  QBuffer buf;
  buf.setData(section->data());
  buf.open(QIODevice::ReadOnly);
  Reader reader(buf);

  targetAddr = reader.pos();
  const auto target = Util::decodeMacSdkVersion(reader.getUInt32());

  sdkAddr = reader.pos();
  const auto sdk = Util::decodeMacSdkVersion(reader.getUInt32());

  targetMajor = std::get<0>(target);
  targetMajorSpin->setValue(targetMajor);

  targetMinor = std::get<1>(target);
  targetMinorSpin->setValue(targetMinor);

  sdkMajor = std::get<0>(sdk);
  sdkMajorSpin->setValue(sdkMajor);

  sdkMinor = std::get<1>(sdk);
  sdkMinorSpin->setValue(sdkMinor);
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
