#include <QComboBox>
#include <QDebug>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>

#include "Constants.h"
#include "Util.h"

#include "widgets/AsciiValidator.h"
#include "widgets/ConversionHelper.h"
#include "widgets/NumberValidator.h"

namespace dispar {

ConversionHelper::ConversionHelper(QWidget *parent) : QDialog(parent)
{
  setWindowTitle(tr("Conversion Helper"));
  createLayout();
  resize(300, 550);
}

void ConversionHelper::onTextEdited(const QString &text)
{
  auto *edit = qobject_cast<QLineEdit *>(sender());
  if (!edit) return;

  quint64 val{0};
  bool ok{true};

  for (int i = 0; i < edits.size(); i++) {
    auto *e = edits[i];
    if (e != edit) continue;

    // Oct
    if (i == 0) {
      val = text.toULongLong(&ok, 8);
    }

    // Dec
    else if (i == 1) {
      val = text.toULongLong(&ok, 10);
    }

    // Hex
    else if (i == 2) {
      val = text.toULongLong(&ok, 16);
    }

    // ACSII
    else if (i == 3) {
      if (text.size() == 1) {
        val = static_cast<quint64>(text[0].toLatin1());
      }
    }
  }

  for (int i = 0; i < edits.size(); i++) {
    auto *e = edits[i];
    if (e == edit) continue;

    if (!ok) {
      e->setText("");
      continue;
    }

    // Oct
    if (i == 0) {
      e->setText(QString::number(val, 8));
    }

    // Dec
    else if (i == 1) {
      e->setText(QString::number(val));
    }

    // Hex
    else if (i == 2) {
      e->setText(QString::number(val, 16).toUpper());
    }

    // ACSII
    else if (i == 3 && val > 0) {
      if (val < 32 || val > 126) {
        e->setText("");
        e->setPlaceholderText(tr("None"));
      }
      else {
        e->setText(QString((char) val));
      }
    }
  }
}

void ConversionHelper::onHexToText()
{
  auto hex = hexEdit->toPlainText();
  if (hex.trimmed().replace(" ", "").isEmpty()) {
    return;
  }

  bool unicode = (encBox->currentIndex() == 1);

  const auto text = [&] {
    if (unicode) {
      return Util::hexToUnicode(hex);
    }

    hex = hex.trimmed().replace(" ", "");
    return Util::hexToAscii(hex, 0, hex.size() / 2);
  }();

  if (text.isEmpty()) {
    QMessageBox::information(this, "dispar", tr("Could not convert hex to text."));
  }
  textEdit->setText(text);
}

void ConversionHelper::onTextToHex()
{
  auto text = textEdit->toPlainText().trimmed();
  if (text.isEmpty()) return;

  bool unicode = (encBox->currentIndex() == 1);

  QString hex;
  for (const auto &c : text) {
    int ic;
    if (unicode) {
      ic = c.unicode();
    }
    else {
      ic = c.toLatin1();
    }
    hex += Util::padString(QString::number(ic, 16).toUpper(), 2) + " ";
  }
  if (hex.isEmpty()) {
    QMessageBox::information(this, "dispar", tr("Could not convert text to hex."));
  }
  hexEdit->setText(hex);
}

void ConversionHelper::createLayout()
{
  auto *numLayout = new QGridLayout;
  numLayout->setContentsMargins(5, 5, 5, 5);
  numLayout->addWidget(new QLabel(tr("Octal")), 0, 0);
  numLayout->addWidget(new QLabel(tr("Decimal")), 1, 0);
  numLayout->addWidget(new QLabel(tr("Hexadecimal")), 2, 0);
  numLayout->addWidget(new QLabel(tr("ASCII")), 3, 0);

  for (int i = 0; i < 4; i++) {
    auto *edit = new QLineEdit;
    connect(edit, &QLineEdit::textEdited, this, &ConversionHelper::onTextEdited);

    if (i == 0) {
      edit->setValidator(new NumberValidator(8, this));
    }
    else if (i == 1) {
      edit->setValidator(new NumberValidator(10, this));
    }
    else if (i == 2) {
      edit->setValidator(new NumberValidator(16, this));
    }
    else if (i == 3) {
      edit->setMaxLength(1);
      edit->setValidator(new AsciiValidator(this));
    }

    numLayout->addWidget(edit, i, 1);
    edits << edit;
  }

  auto *numGroup = new QGroupBox(tr("Numbers"));
  numGroup->setLayout(numLayout);

  hexEdit = new QTextEdit;
  hexEdit->setTabChangesFocus(true);
  hexEdit->setFont(Constants::FIXED_FONT);

  textEdit = new QTextEdit;
  textEdit->setTabChangesFocus(true);
  textEdit->setFont(Constants::FIXED_FONT);

  encBox = new QComboBox;
  encBox->addItem(tr("ASCII"), 0);
  encBox->addItem(tr("Unicode"), 1);

  auto *encLayout = new QHBoxLayout;
  encLayout->addWidget(new QLabel(tr("Encoding:")));
  encLayout->addWidget(encBox);
  encLayout->addStretch();

  auto *hexToText = new QPushButton(tr("Hex -> Text"));
  connect(hexToText, &QPushButton::clicked, this, &ConversionHelper::onHexToText);

  auto *textToHex = new QPushButton(tr("Text -> Hex"));
  connect(textToHex, &QPushButton::clicked, this, &ConversionHelper::onTextToHex);

  auto *buttonLayout = new QHBoxLayout;
  buttonLayout->setContentsMargins(0, 0, 0, 0);
  buttonLayout->addStretch();
  buttonLayout->addWidget(hexToText);
  buttonLayout->addWidget(textToHex);
  buttonLayout->addStretch();

  auto *textLayout = new QVBoxLayout;
  textLayout->setContentsMargins(5, 5, 5, 5);
  textLayout->addWidget(new QLabel(tr("Hex strings")));
  textLayout->addWidget(hexEdit);
  textLayout->addWidget(new QLabel(tr("Text")));
  textLayout->addWidget(textEdit);
  textLayout->addLayout(encLayout);
  textLayout->addLayout(buttonLayout);

  auto *textGroup = new QGroupBox(tr("Text"));
  textGroup->setLayout(textLayout);

  auto *layout = new QVBoxLayout;
  layout->addWidget(numGroup);
  layout->addWidget(textGroup);

  setLayout(layout);
}

} // namespace dispar
