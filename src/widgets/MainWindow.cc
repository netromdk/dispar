#include "widgets/MainWindow.h"
#include "BinaryObject.h"
#include "Context.h"
#include "Project.h"
#include "Util.h"
#include "Version.h"
#include "formats/Format.h"
#include "formats/FormatLoader.h"
#include "widgets/AboutDialog.h"
#include "widgets/BinaryWidget.h"
#include "widgets/ConversionHelper.h"
#include "widgets/DisassemblerDialog.h"
#include "widgets/LogDialog.h"
#include "widgets/OptionsDialog.h"

#include <cassert>

#include <QApplication>
#include <QCloseEvent>
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QFileDialog>
#include <QInputDialog>
#include <QLabel>
#include <QMenuBar>
#include <QMessageBox>
#include <QProgressDialog>
#include <QSet>
#include <QVBoxLayout>

namespace dispar {

MainWindow::MainWindow(const QString &file)
  : modified(false), binaryModified(false), startupFile(file)
{
  setTitle();
  createLayout();
  createMenu();
}

MainWindow::~MainWindow()
{
  Context::get().setValue("MainWindow.geometry", Util::byteArrayString(saveGeometry()));
}

void MainWindow::showEvent(QShowEvent *event)
{
  QMainWindow::showEvent(event);

  static bool first = true;
  if (!first) return;
  first = true;

  if (!restoreGeometry(Util::byteArray(Context::get().value("MainWindow.geometry").toString()))) {
    Util::resizeRatioOfScreen(this, 0.7);
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

void MainWindow::closeEvent(QCloseEvent *event)
{
  if (!checkSave()) {
    event->ignore();
    return;
  }
  else {
    modified = false;
    event->accept();
  }

  QMainWindow::closeEvent(event);
}

void MainWindow::newProject()
{
  if (!checkSave()) return;

  // Reset modified state so closeProject() doesn't ask to save as well.
  modified = false;
  closeProject();

  openBinary();
}

void MainWindow::openProject(const QString &projectFile)
{
  if (!checkSave()) return;

  QString file = projectFile;
  if (file.isEmpty()) {
    QFileDialog diag(this, tr("Open Project"), QDir::homePath());
    diag.setNameFilters(QStringList{"Dispar project (*.dispar)"});

    if (!diag.exec()) return;

    file = diag.selectedFiles().first();
  }

  qDebug() << "Opening project:" << file;
  closeProject();

  auto *project = Context::get().loadProject(file);
  if (!project) {
    modified = false;
    QMessageBox::critical(this, "dispar",
                          tr("Failed to load \"%1\"!\nMake sure file is valid.").arg(file));
    return;
  }

  auto binary = project->binary();
  if (!QFile::exists(binary)) {
    auto ret = QMessageBox::question(
      this, "dispar",
      tr("Binary of project does not exist: %1!\n\nDo you want to load another "
         "project or choose another binary for this project?")
        .arg(binary),
      QMessageBox::Yes | QMessageBox::Open | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Yes:
      openProject();
      return;

    case QMessageBox::Open:
      openBinary();
      return;

    default:
    case QMessageBox::Cancel:
      return;
    }
  }

  connect(project, &Project::modified, this, &MainWindow::onProjectModified);

  // Add recent file.
  Context::get().addRecentProject(file);

  loadBinary(binary);
}

bool MainWindow::saveProject()
{
  bool saveAs = (sender() == saveAsProjectAction);

  auto project = Context::get().project();
  assert(project);

  auto ask = project->file().isEmpty() || saveAs;

  QString saveToFile; ///< Empty means use existing file name.
  if (ask) {
    saveToFile = QFileDialog::getSaveFileName(this, tr("Save Project"), QDir::homePath(),
                                              tr("Dispar project (*.dispar)"));
    if (saveToFile.isEmpty()) {
      QMessageBox::warning(this, "dispar", tr("Project not saved!"));
      return false;
    }
  }

  // Attach all modified regions before saving.
  project->clearModifiedRegions();
  for (const auto *object : format->objects()) {
    for (const auto *section : object->sections()) {
      if (!section->isModified()) {
        continue;
      }

      const auto &data = section->data();
      for (const auto &region : section->modifiedRegions()) {
        project->addModifiedRegion(section->offset() + region.first,
                                   data.mid(region.first, region.second));
      }
    }
  }

  if (!project->save(saveToFile)) {
    QMessageBox::critical(this, "dispar", tr("Could not save project!"));
    return false;
  }

  modified = false;
  setTitle(project->file());
  return true;
}

void MainWindow::closeProject()
{
  if (!checkSave()) return;

  Context::get().clearProject();

  modified = false;
  binaryModified = false;
  setTitle();

  newProjectAction->setEnabled(false);
  saveProjectAction->setEnabled(false);
  saveAsProjectAction->setEnabled(false);
  closeProjectAction->setEnabled(false);
  saveBinaryAction->setEnabled(false);
  reloadBinaryAction->setEnabled(false);

  if (centralWidget()) {
    centralWidget()->deleteLater();
  }

  createLayout();
}

void MainWindow::openBinary()
{
  QFileDialog diag(this, tr("Open Binary"), QDir::homePath());
  diag.setNameFilters({"Mach-O binary (*.o *.dylib *.dylinker *.bundle *.app *)", "Any file (*)"});

  if (!diag.exec()) return;

  auto file = diag.selectedFiles().first();
  loadBinary(file);
}

bool MainWindow::saveBinary()
{
  auto &ctx = Context::get();
  if (ctx.backupEnabled()) {
    saveBackup(format->file());
  }

  QFile f(format->file());
  if (!f.open(QIODevice::ReadWrite)) {
    QMessageBox::critical(this, "",
                          tr("Could not open binary file for writing: %1").arg(f.fileName()));
    return false;
  }

  qDebug() << "Committing modified regions to binary:" << format->file();
  format->write(f);

  binaryModified = false;
  saveBinaryAction->setEnabled(false);
  setTitle(Context::get().project()->file());
  return true;
}

void MainWindow::reloadBinary()
{
  if (!checkSave()) return;
  if (!checkSaveBinary()) return;
  loadBinary(Context::get().project()->binary());
}

void MainWindow::onRecentProject()
{
  auto *action = qobject_cast<QAction *>(sender());
  if (!action) return;
  openProject(action->text());
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

void MainWindow::onLog()
{
  static auto *diag = new LogDialog(this);
  diag->show();
  diag->raise();
  diag->activateWindow();
}

void MainWindow::onLoadSuccess(std::shared_ptr<Format> fmt)
{
  format = fmt;

  // If no project was active then create one, and if a project was not already saved then reset it.
  auto &ctx = Context::get();
  auto *project = ctx.project();
  if (!project || project->file().isEmpty()) {
    project = ctx.resetProject();
    connect(project, &Project::modified, this, &MainWindow::onProjectModified);
  }

  project->setBinary(fmt->file());
  newProjectAction->setEnabled(true);

  binaryModified = false;
  setTitle(project->file());

  saveProjectAction->setEnabled(true);
  saveAsProjectAction->setEnabled(true);
  closeProjectAction->setEnabled(true);
  saveBinaryAction->setEnabled(false);
  reloadBinaryAction->setEnabled(true);

  Util::delayFunc([this, fmt] {
    auto file = fmt->file();

    // Add recent file.
    Context::get().addRecentBinary(file);

    auto objects = fmt->objects();
    BinaryObject *object = nullptr;
    if (objects.size() == 1) {
      object = objects.first();
    }
    else {
      auto currentCpu = Util::currentCpuType();
      qDebug() << "Current ARCH:" << cpuTypeName(currentCpu);

      QStringList items;
      int current = 0;
      for (int i = 0; i < objects.size(); i++) {
        auto *obj = objects[i];
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
      assert(idx != -1);
      object = objects[idx];
    }
    assert(object);

    applyModifiedRegions(object);

    QElapsedTimer elapsedTimer;
    elapsedTimer.start();

    QProgressDialog disDiag(this);
    disDiag.setLabelText(tr("Disassembling code sections.."));
    disDiag.setCancelButton(nullptr);
    disDiag.setRange(0, 0);
    disDiag.show();
    qApp->processEvents();
    qDebug() << qPrintable(disDiag.labelText());

    Disassembler dis(*object, Context::get().disassemblerSyntax());
    if (dis.valid()) {
      for (auto &sec : object->sections()) {
        switch (sec->type()) {
        case Section::Type::TEXT:
        case Section::Type::SYMBOL_STUBS: {
          auto res = dis.disassemble(sec->data());
          if (res) {
            sec->setDisassembly(std::move(res));
          }
          break;
        }

        default:
          break;
        }
      }
    }

    disDiag.close();
    qDebug() << ">" << elapsedTimer.restart() << "ms";

    if (centralWidget()) {
      centralWidget()->deleteLater();
    }

    auto *binaryWidget = new BinaryWidget(object);
    connect(binaryWidget, &BinaryWidget::modified, this, &MainWindow::onBinaryModified);

    setCentralWidget(binaryWidget);
  });
}

void MainWindow::onProjectModified()
{
  modified = true;
  setTitle(Context::get().project()->file());
}

void MainWindow::onBinaryModified()
{
  binaryModified = true;
  saveBinaryAction->setEnabled(true);
  setTitle(Context::get().project()->file());
}

void MainWindow::setTitle(const QString &file)
{
  setWindowTitle(QString("Dispar v%1%2%3%4")
                   .arg(versionString())
                   .arg(!file.isEmpty() ? " - " + file : "")
                   .arg(modified ? " *" : "")
                   .arg(binaryModified ? " (" + tr("BINARY CHANGES PENDING") + ")" : ""));
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
  auto &ctx = Context::get();
  auto *fileMenu = menuBar()->addMenu(tr("&File"));

  newProjectAction =
    fileMenu->addAction(tr("New project"), this, SLOT(newProject()), QKeySequence::New);
  newProjectAction->setEnabled(false);

  fileMenu->addAction(tr("Open project"), this, SLOT(openProject()), QKeySequence::Open);
  const auto recentProjects = ctx.recentProjects();
  if (!recentProjects.isEmpty()) {
    auto *recentMenu = fileMenu->addMenu(tr("Open recent projects"));
    for (const auto &file : recentProjects) {
      recentMenu->addAction(file, this, SLOT(onRecentProject()));
    }
  }

  fileMenu->addSeparator();

  fileMenu->addAction(tr("Open binary"), this, SLOT(openBinary()),
                      QKeySequence(Qt::ALT + Qt::CTRL + Qt::Key_O));

  const auto recentBinaries = ctx.recentBinaries();
  if (!recentBinaries.isEmpty()) {
    auto *recentMenu = fileMenu->addMenu(tr("Open recent binaries"));
    for (const auto &file : recentBinaries) {
      recentMenu->addAction(file, this, SLOT(onRecentBinary()));
    }
  }

  saveBinaryAction = fileMenu->addAction(tr("Save binary"), this, SLOT(saveBinary()),
                                         QKeySequence(Qt::ALT + Qt::CTRL + Qt::Key_S));
  saveBinaryAction->setEnabled(false);

  reloadBinaryAction = fileMenu->addAction(tr("Reload binary"), this, SLOT(reloadBinary()),
                                           QKeySequence(Qt::ALT + Qt::CTRL + Qt::Key_R));
  reloadBinaryAction->setEnabled(false);

  fileMenu->addSeparator();

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

  auto *toolsMenu = menuBar()->addMenu(tr("&Tools"));
  toolsMenu->addAction(tr("Conversion helper"), this, SLOT(onConversionHelper()),
                       QKeySequence(Qt::SHIFT + Qt::CTRL + Qt::Key_C));
  toolsMenu->addAction(tr("Disassembler"), this, SLOT(onDisassembler()),
                       QKeySequence(Qt::SHIFT + Qt::CTRL + Qt::Key_D));

  auto *helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction(tr("&About"), this, SLOT(onAbout()));
  helpMenu->addAction(tr("&Options"), this, SLOT(onOptions()));
  helpMenu->addAction(tr("&Log"), this, SLOT(onLog()));
}

void MainWindow::loadBinary(QString file)
{
  // Resolve absolute path.
  file = QFileInfo(file).absoluteFilePath();

  // If .app then resolve the internal binary file.
  file = Util::resolveAppBinary(file);

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

void MainWindow::saveBackup(const QString &file)
{
  // Determine if prior backups have been made and, if so, how many.
  QFileInfo fi(file);
  auto dir = fi.dir();
  int bakCount = 0, bakNum = 0;
  QStringList files;
  const auto entries = dir.entryInfoList(QStringList{QString("%1.bak*").arg(fi.fileName())},
                                         QDir::Files | QDir::Hidden, QDir::Name);
  for (const auto &entry : entries) {
    files << entry.absoluteFilePath();
    bakCount++;
    const auto ext = entry.suffix();
    static QRegExp re("bak([\\d]+)$");
    if (re.indexIn(ext) != -1 && re.captureCount() == 1) {
      bakNum = re.capturedTexts()[1].toUInt();
    }
  }

  // Remove previous backups if not unlimited. And remove one due to
  // the file that will be created beneath.
  auto &ctx = Context::get();
  const auto maxAmount = ctx.backupAmount();
  if (maxAmount > 0 && bakCount >= maxAmount - 1) {
    for (int i = 0; i < bakCount - (maxAmount - 1); i++) {
      QFile::remove(files[i]);
    }
  }

  const auto num = Util::padString(QString::number(++bakNum), 4),
             dest = QString("%1.bak%2").arg(file).arg(num);
  qDebug() << "Saving backup of" << file << "to" << dest;
  if (!QFile::copy(file, dest)) {
    QMessageBox::warning(this, "", tr("Could not save backup to \"%1\"!").arg(dest));
  }
}

bool MainWindow::checkSave()
{
  if (!modified) return true;

  auto ret = QMessageBox::question(this, "dispar", tr("Project modified. Save it first?"),
                                   QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
  if (QMessageBox::Yes == ret) {
    return saveProject();
  }
  else if (QMessageBox::Cancel == ret) {
    return false;
  }

  return true;
}

bool MainWindow::checkSaveBinary()
{
  if (!binaryModified) return true;

  auto ret = QMessageBox::question(this, "dispar", tr("Binary modified. Save it first?"),
                                   QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
  if (QMessageBox::Yes == ret) {
    return saveBinary();
  }
  else if (QMessageBox::Cancel == ret) {
    return false;
  }

  return true;
}

void MainWindow::applyModifiedRegions(BinaryObject *object)
{
  auto project = Context::get().project();
  const auto &modifiedRegions = project->modifiedRegions();
  if (modifiedRegions.isEmpty()) {
    return;
  }

  bool match = false;
  for (auto *section : object->sections()) {
    for (const auto addr : modifiedRegions.keys()) {
      const auto &data = modifiedRegions[addr];
      if (addr >= section->offset() && addr + data.size() < section->offset() + section->size()) {
        // Make sure the region hasn't already been written to the binary. The binary thus wouldn't
        // be modified in that case.
        const auto pos = addr - section->offset();
        if (data != section->data().mid(pos, data.size())) {
          section->setSubData(data, pos);
          match = true;
        }
      }
    }
  }

  // The modified regions are no longer needed in the project.
  project->clearModifiedRegions();

  if (match) {
    onBinaryModified();
  }
}

} // namespace dispar
