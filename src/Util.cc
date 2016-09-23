#include "Util.h"

#include <QTimer>
#include <QScrollBar>
#include <QDir>
#include <QFileInfo>
#include <QDesktopWidget>
#include <QApplication>
#include <QAbstractScrollArea>

#ifdef HAS_LIBIBERTY
#include <libiberty.h>
#include <demangle.h>
#else
#include <stdlib.h>
#include <cxxabi.h>
#endif

QString Util::formatSize(qint64 bytes, int digits)
{
  constexpr double KB = 1024, MB = 1024 * KB, GB = 1024 * MB, TB = 1024 * GB;
  QString unit{"B"};
  double size = bytes;
  if (size >= TB) {
    size /= TB;
    unit = "TB";
  }
  else if (size >= GB) {
    size /= GB;
    unit = "GB";
  }
  else if (size >= MB) {
    size /= MB;
    unit = "MB";
  }
  else if (size >= KB) {
    size /= KB;
    unit = "KB";
  }
  return QString("%1 %2").arg(QString::number(size, 'f', digits)).arg(unit);
}

QString Util::padString(const QString &str, int size, bool before, char pad)
{
  int addrLen = str.size();
  if (addrLen < size) {
    QString res{str}, padStr{QString(size - addrLen, pad)};
    return (before ? padStr + res : res + padStr);
  }
  return str;
}

QString Util::dataToAscii(const QByteArray &data, int offset, int size)
{
  QString res;
  int len = data.size();
  for (int i = offset; i - offset < size && i < len; i++) {
    int ic = data[i];
    res += (ic >= 32 && ic <= 126 ? (char) ic : '.');
  }
  return res;
}

QString Util::hexToAscii(const QString &str, int offset, int blocks, bool unicode)
{
  QString res;
  int len = str.size();
  int size = blocks * 2;
  bool ok;
  for (int i = offset; i - offset <= size && i < len; i += 2) {
    if (str[i] == ' ') {
      size++;
      i--;
      continue;
    }
    int ic = str.mid(i, 2).toInt(&ok, 16);
    if (!ok) return QString();
    if (!unicode) {
      res += (ic >= 32 && ic <= 126 ? (char) ic : '.');
    }
    else {
      res += QString::fromUtf16((ushort *) &ic, 1);
    }
  }
  return res;
}

QString Util::hexToString(const QString &str)
{
  return QString::fromUtf8(hexToData(str));
}

QByteArray Util::hexToData(const QString &str)
{
  QByteArray data;
  int size = str.size();
  bool ok;
  for (int i = 0; i < size; i += 2) {
    data += str.mid(i, 2).toInt(&ok, 16);
    if (!ok) return QByteArray();
  }
  return data;
}

QString Util::resolveAppBinary(QString path)
{
  if (path.endsWith("/")) {
    path.chop(1);
  }
  if (!path.toLower().endsWith(".app")) {
    return QString();
  }
  QDir dir(path);
  if (dir.exists() && dir.cd("Contents") && dir.cd("MacOS")) {
    QFileInfo fi(path);
    if (dir.exists(fi.baseName())) {
      return dir.absoluteFilePath(fi.baseName());
    }
  }
  return QString();
}

void Util::centerWidget(QWidget *widget)
{
  widget->move(QApplication::desktop()->screen()->rect().center() - widget->rect().center());
}

QString Util::addrDataString(quint64 addr, QByteArray data)
{
  // Pad data to a multiple of 16.
  quint64 rest = data.size() % 16;
  if (rest != 0) {
    int amount = 16 - rest;
    for (int i = 0; i < amount; i++) {
      data += (char) 0;
    }
  }

  QString output = QString::number(addr, 16).toUpper() + ": ";
  QString ascii;
  for (int i = 0; i < data.size(); i++) {
    char c = data[i];
    int ic = c;
    unsigned char uc = c;
    QString hc = QString::number(uc, 16).toUpper();
    if (hc.size() == 1) {
      hc = "0" + hc;
    }
    output += hc + " ";
    ascii += (ic >= 33 && ic <= 126 ? c : '.');
    if ((i + 1) % 16 == 0 || i == data.size() - 1) {
      output += "  " + ascii;
      ascii.clear();
      if (i != data.size() - 1) {
        addr += 16;
        output += "\n" + QString::number(addr, 16).toUpper() + ": ";
      }
    }
  }
  return output;
}

void Util::scrollToTop(QAbstractScrollArea *widget)
{
  QTimer::singleShot(1, widget, [widget] {
    auto *bar = widget->verticalScrollBar();
    if (bar) {
      bar->triggerAction(QScrollBar::SliderToMinimum);
    }
  });
}

QString Util::demangle(const QString &name)
{
  if (name.isEmpty()) {
    return name;
  }

  // Skip leading . or $.
  int skip = 0;
  if (name[0] == '.' || name[0] == '$') {
    skip++;
  }

  // Skip leading underscore.
  if (name.size() > 1 && name[skip] == '_') {
    skip++;
  }

  const auto *mangledName = name.mid(skip).toUtf8().constData();

#ifdef HAS_LIBIBERTY
  int flags = DMGL_PARAMS | DMGL_ANSI | DMGL_VERBOSE;
  auto *res = cplus_demangle(mangledName, flags);
  if (res == 0) {
    return name;
  }
#else
  int status;
  auto *res = abi::__cxa_demangle(mangledName, NULL, NULL, &status);
  if (status != 0) {
    return name;
  }
#endif

  auto output = QString::fromUtf8(res);
  free(res);
  return output;
}
