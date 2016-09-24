#include "AboutDialog.h"
#include "../Version.h"

#include <QLabel>

#include <capstone.h>

AboutDialog::AboutDialog(QWidget *parent) : QDialog(parent)
{
  setWindowTitle(tr("About Dispar"));
  createLayout();
}

void AboutDialog::createLayout()
{
  auto *titleLabel = new QLabel("Dispar", this);
  auto font = titleLabel->font();
  font.setPointSize(font.pointSize() + 10);
  font.setBold(true);
  titleLabel->setFont(font);
  titleLabel->move(20, 20);

  auto *versionLabel = new QLabel(QString("v. %1").arg(versionString()), this);
  versionLabel->move(260, 30);

  int yPos = titleLabel->pos().y() + titleLabel->height() + 15;

  auto addDep = [this, &yPos](const QString &name, const QString &version, const QString &link) {
    auto *label =
      new QLabel(QString("%1 %2 - <a href=\"%3\">%3</a>").arg(name).arg(version).arg(link), this);
    label->setTextFormat(Qt::RichText);
    label->setTextInteractionFlags(Qt::TextBrowserInteraction);
    label->setOpenExternalLinks(true);
    label->move(20, yPos);
    yPos += label->height();
  };

  addDep("Qt", QT_VERSION_STR, "https://www.qt.io");
  addDep("Capstone", QString("%1.%2").arg(CS_API_MAJOR).arg(CS_API_MINOR),
         "http://www.capstone-engine.org");
  addDep("libiberty", "2.27", "https://www.gnu.org/software/binutils");
}
