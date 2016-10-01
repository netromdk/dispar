#include "MainWindow.h"
#include "../BinaryObject.h"
#include "../Context.h"
#include "../Project.h"
#include "../Util.h"
#include "../Version.h"
#include "../formats/Format.h"
#include "../formats/FormatLoader.h"
#include "AboutDialog.h"
#include "BinaryWidget.h"
#include "ConversionHelper.h"
#include "DisassemblerDialog.h"
#include "OptionsDialog.h"

#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QInputDialog>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QProgressDialog>
#include <QSet>
#include <QSettings>
#include <QVBoxLayout>

MainWindow::MainWindow(const QString &file) : modified(false), startupFile(file)
{
  setTitle();
  readSettings();
  createLayout();
  createMenu();
}

MainWindow::~MainWindow()
{
  QSettings settings;
  settings.setValue("MainWindow.geometry", saveGeometry());
  settings.setValue("MainWindow.recentBinaries", recentBinaries);
}

void MainWindow::showEvent(QShowEvent *event)
{
  QMainWindow::showEvent(event);

  static bool first = true;
  if (!first) return;
  first = true;

  if (!restoreGeometry(geometry)) {
    resize(900, 500);
    Util::centerWidget(this);
  }

  // Load specified files or open file dialog.
  Util::delayFunc([this] {
    if (!startupFile.isEmpty()) {
      if (startupFile.toLower().endsWith(".dispar")) {
        openProject(startupFile);
      }
      else {
        loadBinary(startupFile);
      }
    }
  });
}

void MainWindow::openProject(const QString &projectFile)
{
  // TODO: Ask to save if project is live and unsaved!

  QString file = projectFile;
  if (file.isEmpty()) {
    QFileDialog diag(this, tr("Open Project"), QDir::homePath());
    diag.setNameFilters(QStringList{"Dispar project (*.dispar)"});

    if (!diag.exec()) return;

    file = diag.selectedFiles().first();
  }

  qDebug() << "Opening project:" << file;
  closeProject();

  auto project = Context::get().loadProject(file);
  if (!project) {
    QMessageBox::critical(this, "dispar",
                          tr("Failed to load \"%1\"!\nMake sure file is valid.").arg(file));
    return;
  }

  // TODO: Add to recent projects list.

  loadBinary(project->binary());
}

void MainWindow::saveProject()
{
  bool saveAs = (sender() == saveAsProjectAction);

  auto project = Context::get().project();
  Q_ASSERT(project);

  auto ask = project->file().isEmpty() || saveAs;

  bool ret;
  if (ask) {
    QString file = QFileDialog::getSaveFileName(this, tr("Save Project"), QDir::homePath(),
                                                tr("Dispar project (*.dispar)"));
    if (file.isEmpty()) {
      QMessageBox::warning(this, "dispar", tr("Project not saved!"));
      return;
    }

    ret = project->save(file);
  }
  else {
    ret = project->save();
  }

  if (!ret) {
    QMessageBox::critical(this, "dispar", tr("Could not save project!"));
  }
}

void MainWindow::closeProject()
{
  // TODO: Ask to save if project is live and unsaved.
  Context::get().clearProject();

  saveProjectAction->setEnabled(false);
  saveAsProjectAction->setEnabled(false);
  closeProjectAction->setEnabled(false);

  centralWidget()->deleteLater();
  createLayout();
}

void MainWindow::openBinary()
{
  QFileDialog diag(this, tr("Open Binary"), QDir::homePath());
  diag.setNameFilters(
    QStringList{"Mach-O binary (*.o *.dylib *.dylinker *.bundle *.app *)", "Any file (*)"});

  if (!diag.exec()) return;

  auto file = diag.selectedFiles().first();
  loadBinary(file);
}

void MainWindow::onRecentBinary()
{
  auto *action = qobject_cast<QAction *>(sender());
  if (!action) return;
  loadBinary(action->text());
}

void MainWindow::onConversionHelper()
{
  static auto *diag = new ConversionHelper(this);
  diag->show();
  diag->raise();
  diag->activateWindow();
}

void MainWindow::onDisassembler()
{
  static auto *diag = new DisassemblerDialog(this, Util::currentCpuType());
  diag->show();
  diag->raise();
  diag->activateWindow();
}

void MainWindow::onAbout()
{
  AboutDialog diag;
  diag.exec();
}

void MainWindow::onOptions()
{
  OptionsDialog diag;
  diag.exec();
}

void MainWindow::onLoadSuccess(std::shared_ptr<Format> fmt)
{
  // If no project was active then create one, and if a project was not already saved then reset it.
  auto &ctx = Context::get();
  auto project = ctx.project();
  if (!project || project->file().isEmpty()) {
    project = ctx.resetProject();
  }
  project->setBinary(fmt->file());

  saveProjectAction->setEnabled(true);
  saveAsProjectAction->setEnabled(true);
  closeProjectAction->setEnabled(true);

  Util::delayFunc([this, fmt] {
    auto file = fmt->file();

    // Add recent file.
    if (!recentBinaries.contains(file)) {
      recentBinaries << file;
    }
    if (recentBinaries.size() > 10) {
      recentBinaries.removeFirst();
    }

    auto objects = fmt->objects();
    std::shared_ptr<BinaryObject> object = nullptr;
    if (objects.size() == 1) {
      object = objects.first();
    }
    else {
      auto currentCpu = Util::currentCpuType();
      qDebug() << "Current ARCH:" << cpuTypeName(currentCpu);

      QStringList items;
      int current = 0;
      for (int i = 0; i < objects.size(); i++) {
        auto &obj = objects[i];
        items << QString("%1, %2 (%3-bit)")
                   .arg(cpuTypeName(obj->cpuType()))
                   .arg(cpuTypeName(obj->cpuSubType()))
                   .arg(obj->systemBits());

        if (obj->cpuType() == currentCpu && obj->systemBits() == sizeof(void *) * 8) {
          current = i;
        }
      }

      bool ok;
      auto choice = QInputDialog::getItem(this, tr("%1 binary objects in file").arg(objects.size()),
                                          tr("Choose:"), items, current, false, &ok);

      if (!ok || choice.isEmpty()) {
        return;
      }

      int idx = items.indexOf(choice);
      Q_ASSERT(idx != -1);
      object = objects[idx];
    }
    Q_ASSERT(object);

    setTitle(file, object->cpuType());

    QProgressDialog disDiag(this);
    disDiag.setLabelText(tr("Disassembling code sections.."));
    disDiag.setCancelButton(nullptr);
    disDiag.setRange(0, 0);
    disDiag.show();
    qApp->processEvents();
    qDebug() << qPrintable(disDiag.labelText());

    Disassembler dis(object, Context::get().disassemblerSyntax());
    if (dis.valid()) {
      for (auto &sec : object->sections()) {
        switch (sec->type()) {
        case Section::Type::TEXT:
        case Section::Type::SYMBOL_STUBS: {
          auto res = dis.disassemble(sec->data());
          if (res) {
            sec->setDisassembly(res);
          }
          break;
        }

        default:
          break;
        }
      }
    }

    disDiag.close();

    if (centralWidget()) {
      centralWidget()->deleteLater();
    }
    setCentralWidget(new BinaryWidget(object));
  });
}

void MainWindow::setTitle(const QString &file, CpuType type)
{
  setWindowTitle(
    QString("Dispar v%1%2")
      .arg(versionString())
      .arg(!file.isEmpty() ? QString(" - %1 (%2)").arg(file).arg(cpuTypeName(type)) : ""));
}

void MainWindow::readSettings()
{
  QSettings settings;
  geometry = settings.value("MainWindow.geometry").toByteArray();

  // Load recent binaries.
  recentBinaries = settings.value("MainWindow.recentBinaries", QStringList()).toStringList();
  for (int i = recentBinaries.size() - 1; i >= 0; i--) {
    if (!QFile::exists(recentBinaries[i])) {
      recentBinaries.removeAt(i);
    }
  }
  if (recentBinaries.size() > 10) {
    recentBinaries = recentBinaries.mid(recentBinaries.size() - 10);
  }
}

void MainWindow::createLayout()
{
  auto *label = new QLabel(tr("Open a project file or load a binary!"));
  label->setAlignment(Qt::AlignCenter);

  auto font = label->font();
  font.setPointSize(24);
  font.setBold(true);
  label->setFont(font);

  setCentralWidget(label);
}

void MainWindow::createMenu()
{
  auto *fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(tr("Open project"), this, SLOT(openProject()), QKeySequence::Open);
  // TODO: Put recent project files here.

  saveProjectAction =
    fileMenu->addAction(tr("Save project"), this, SLOT(saveProject()), QKeySequence::Save);
  saveProjectAction->setEnabled(false);

  saveAsProjectAction =
    fileMenu->addAction(tr("Save as.."), this, SLOT(saveProject()), QKeySequence::SaveAs);
  saveAsProjectAction->setEnabled(false);

  closeProjectAction =
    fileMenu->addAction(tr("Close project"), this, SLOT(closeProject()), QKeySequence::Close);
  closeProjectAction->setEnabled(false);

  fileMenu->addSeparator();

  fileMenu->addAction(tr("Open binary"), this, SLOT(openBinary()),
                      QKeySequence(Qt::SHIFT + Qt::CTRL + Qt::Key_O));
  if (!recentBinaries.isEmpty()) {
    auto *recentMenu = fileMenu->addMenu(tr("Open recent binaries"));
    for (const auto &file : recentBinaries) {
      recentMenu->addAction(file, this, SLOT(onRecentBinary()));
    }
  }

  auto *toolsMenu = menuBar()->addMenu(tr("&Tools"));
  toolsMenu->addAction(tr("Conversion helper"), this, SLOT(onConversionHelper()),
                       QKeySequence(Qt::SHIFT + Qt::CTRL + Qt::Key_C));
  toolsMenu->addAction(tr("Disassembler"), this, SLOT(onDisassembler()),
                       QKeySequence(Qt::SHIFT + Qt::CTRL + Qt::Key_D));

  auto *helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction(tr("&About"), this, SLOT(onAbout()));
  helpMenu->addAction(tr("&Options"), this, SLOT(onOptions()));
}

void MainWindow::loadBinary(QString file)
{
  // Resolve absolute path.
  file = QFileInfo(file).absoluteFilePath();

  // If .app then resolve the internal binary file.
  QString appBin = Util::resolveAppBinary(file);
  if (!appBin.isEmpty()) {
    file = appBin;
  }

  qDebug() << "Loading binary:" << file;

  if (!QFile::exists(file)) {
    qWarning() << "Does not exist!";
    return;
  }

  auto *loaderDiag = new QProgressDialog(this);
  loaderDiag->setLabelText(tr("Detecting format.."));
  loaderDiag->setCancelButton(nullptr);
  loaderDiag->setRange(0, 100);
  loaderDiag->show();
  qDebug() << qPrintable(loaderDiag->labelText());

  loader = std::make_unique<FormatLoader>(file);

  connect(loader.get(), &FormatLoader::failed, this,
          [this](const QString &msg) { QMessageBox::critical(this, "dispar", msg); });

  connect(loader.get(), &FormatLoader::status, this, [loaderDiag](const QString &msg) {
    qDebug() << qPrintable(msg);
    loaderDiag->setLabelText(msg);
  });

  connect(loader.get(), &FormatLoader::progress, this, [diag = loaderDiag](float progress) {
    auto range = diag->maximum() - diag->minimum();
    if (range > 0) {
      auto value = static_cast<float>(range) * progress;
      diag->setValue(value);
    }
  });

  connect(loader.get(), &FormatLoader::success, this, &MainWindow::onLoadSuccess);
  connect(loader.get(), &FormatLoader::finished, this, [this, loaderDiag] {
    qDebug() << "cleaning up loader";
    loader.reset();
    loaderDiag->deleteLater();
  });

  loader->start();
}
