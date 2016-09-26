#include "MainWindow.h"
#include "../BinaryObject.h"
#include "../Util.h"
#include "../Version.h"
#include "../formats/Format.h"
#include "../formats/FormatLoader.h"
#include "AboutDialog.h"
#include "BinaryWidget.h"

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
  settings.setValue("MainWindow.recentFiles", recentFiles);
}

void MainWindow::showEvent(QShowEvent *event)
{
  QMainWindow::showEvent(event);

  static bool first = true;
  if (!first) return;
  first = true;

  if (geometry.isEmpty()) {
    resize(900, 500);
    Util::centerWidget(this);
  }
  else {
    restoreGeometry(geometry);
  }

  // Load specified files or open file dialog.
  Util::delayFunc([this] {
    if (startupFile.isEmpty()) {
      openBinary();
    }
    else {
      loadBinary(startupFile);
    }
  });
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

void MainWindow::onRecentFile()
{
  auto *action = qobject_cast<QAction *>(sender());
  if (!action) return;
  loadBinary(action->text());
}

void MainWindow::onAbout()
{
  AboutDialog diag;
  diag.exec();
}

void MainWindow::onLoadSuccess(std::shared_ptr<Format> fmt)
{
  Util::delayFunc([this, fmt] {
    auto file = fmt->file();

    // Add recent file.
    if (!recentFiles.contains(file)) {
      recentFiles << file;
    }
    if (recentFiles.size() > 10) {
      recentFiles.removeFirst();
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

    Disassembler dis(object);
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

  // Load recent files.
  recentFiles = settings.value("MainWindow.recentFiles", QStringList()).toStringList();
  for (int i = recentFiles.size() - 1; i >= 0; i--) {
    if (!QFile::exists(recentFiles[i])) {
      recentFiles.removeAt(i);
    }
  }
  if (recentFiles.size() > 10) {
    recentFiles = recentFiles.mid(recentFiles.size() - 10);
  }
}

void MainWindow::createLayout()
{
  auto *label = new QLabel(tr("Load a binary file!"));
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
  fileMenu->addAction(tr("Open binary"), this, SLOT(openBinary()), QKeySequence::Open);

  if (!recentFiles.isEmpty()) {
    auto *recentMenu = fileMenu->addMenu(tr("Open recent files"));
    for (const auto &file : recentFiles) {
      recentMenu->addAction(file, this, SLOT(onRecentFile()));
    }
  }

  auto *helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction(tr("&About"), this, SLOT(onAbout()));
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
