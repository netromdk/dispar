#include "MainWindow.h"
#include "../Util.h"
#include "../formats/Format.h"

#include <QDir>
#include <QFileDialog>
#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <QProgressDialog>
#include <QSet>
#include <QSettings>

MainWindow::MainWindow(const QStringList &files)
  : shown(false), modified(false), startupFiles(files)
{
  // Remove possible duplicates.
  startupFiles = startupFiles.toSet().toList();

  setWindowTitle("Dispar");
  /*
  createLayout();
  createMenu();
  */
}

MainWindow::~MainWindow()
{
  QSettings().setValue("MainWindow.geometry", saveGeometry());
}

void MainWindow::showEvent(QShowEvent *event)
{
  QMainWindow::showEvent(event);

  if (shown) return;
  shown = true;

  QSettings settings;
  if (!restoreGeometry(settings.value("MainWindow.geometry").toByteArray())) {
    resize(900, 500);
    Util::centerWidget(this);
  }

  // Load specified files or open file dialog.
  if (startupFiles.isEmpty()) {
    openBinary();
  }
  else {
    for (const auto &file : startupFiles) {
      loadBinary(file);
    }
  }
}

void MainWindow::openBinary()
{
  QFileDialog diag(this, tr("Open Binary"), QDir::homePath());
  diag.setNameFilters(
    QStringList{"Mach-O binary (*.o *.dylib *.dylinker *.bundle *.app *)", "Any file (*)"});

  if (!diag.exec()) {
    if (binaryWidgets.isEmpty()) {
      qApp->quit();
    }
    return;
  }

  auto file = diag.selectedFiles().first();
  /*
  for (const auto *binary : binaryWidgets) {
    if (binary->file() == file) {
      QMessageBox::warning(this, "dispar", tr("Can't open same binary twice!"));
      return;
    }
  }
  */
  loadBinary(file);
}

void MainWindow::loadBinary(QString file)
{
  // If .app then resolve the internal binary file.
  QString appBin = Util::resolveAppBinary(file);
  if (!appBin.isEmpty()) {
    file = appBin;
  }

  qDebug() << "Loading binary:" << file;

  QProgressDialog progDiag(this);
  progDiag.setLabelText(tr("Detecting format.."));
  progDiag.setCancelButton(nullptr);
  progDiag.setRange(0, 0);
  progDiag.show();
  qDebug() << qPrintable(progDiag.labelText());
  qApp->processEvents();

  auto fmt = Format::detect(file);
  if (fmt == nullptr) {
    QMessageBox::critical(this, "dispar", tr("Unknown file - could not open!"));
    return;
  }

  qDebug() << "detected:" << Format::typeName(fmt->type());

  progDiag.setLabelText(tr("Reading and parsing binary.."));
  qDebug() << qPrintable(progDiag.labelText());
  qApp->processEvents();
  if (!fmt->parse()) {
    QMessageBox::warning(this, "dispar", tr("Could not parse file!"));
    return;
  }

  /*
  // Add recent file.
  if (!recentFiles.contains(file)) {
    recentFiles << file;
  }
  if (recentFiles.size() > 10) {
    recentFiles.removeFirst();
  }
  */

  /*
  auto *binWidget = new BinaryWidget(fmt);
  connect(binWidget, &BinaryWidget::modified, this, &MainWindow::onBinaryObjectModified);
  binaryWidgets << binWidget;
  int idx = tabWidget->addTab(binWidget, QFileInfo(file).fileName());
  tabWidget->setCurrentIndex(idx);
  */
}
