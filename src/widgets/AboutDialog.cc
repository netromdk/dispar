#include "widgets/AboutDialog.h"
#include "Constants.h"
#include "Version.h"

#include <QLabel>

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

  yPos = titleLabel->pos().y() + titleLabel->height() + 15;

  using namespace Constants;
  addLinkLabel(PROJECT_URL, PROJECT_URL);

  addDep(Deps::Qt::NAME, Deps::Qt::VERSION, Deps::Qt::URL);
  addDep(Deps::Capstone::NAME, Deps::Capstone::VERSION, Deps::Capstone::URL);
  addDep(Deps::Libiberty::NAME, Deps::Libiberty::VERSION, Deps::Libiberty::URL);
}

void AboutDialog::addLinkLabel(const QString &url, const QString &text, const QString &pretext)
{
  auto *label =
    new QLabel(QString("%3<a href=\"%1\">%2</a>").arg(url).arg(text).arg(pretext), this);
  label->setTextFormat(Qt::RichText);
  label->setTextInteractionFlags(Qt::TextBrowserInteraction);
  label->setOpenExternalLinks(true);
  label->move(20, yPos);
  yPos += label->height();
}

void AboutDialog::addDep(const QString &name, const QString &version, const QString &url)
{
  addLinkLabel(url, url, QString("%1 %2 - ").arg(name).arg(version));
}
