#include "Util.h"

#include <QAbstractScrollArea>
#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QDir>
#include <QFileInfo>
#include <QScrollBar>
#include <QTimer>
#include <QXmlStreamReader>

#include "libiberty/demangle.h"

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

    // cppcheck: No, they are not the same in both branches!
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

QString Util::bytesToHex(const unsigned char *bytes, int size)
{
  QString res;
  for (int j = 0; j < size; j++) {
    res += padString(QString::number(bytes[j], 16), 2, true, '0') + (j < size - 1 ? " " : "");
  }
  return res;
}

QString Util::resolveAppBinary(QString path)
{
  if (path.endsWith("/")) {
    path.chop(1);
  }
  if (!path.toLower().endsWith(".app")) {
    return {};
  }

  QDir dir(path);
  if (!dir.exists() || !dir.cd("Contents")) {
    return {};
  }

  const auto plistFile = dir.absoluteFilePath("Info.plist");

  if (!dir.cd("MacOS")) {
    return {};
  }

  // Try using the name of the app itself without the ".app".
  QFileInfo fi(path);
  if (dir.exists(fi.baseName())) {
    return dir.absoluteFilePath(fi.baseName());
  }

  // Detect the value of "CFBundleExecutable" in "Info.plist".
  // The format is:
  //   <key>CFBundleExecutable</key>
  //   <string>THENAME</string>
  if (!QFile::exists(plistFile)) {
    return {};
  }

  QFile file(plistFile);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return {};
  }

  QXmlStreamReader reader(&file);
  bool nextString = false;
  while (!reader.atEnd()) {
    if (!reader.readNextStartElement()) {
      continue;
    }

    const auto name = reader.name().toString().toLower();
    if (name == "key" && reader.readElementText().toLower() == "cfbundleexecutable") {
      nextString = true;
      continue;
    }

    if (name == "string" && nextString) {
      const auto execName = reader.readElementText();
      if (!dir.exists(execName)) {
        return {};
      }
      return dir.absoluteFilePath(execName);
    }
  }

  return {};
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
#ifdef WIN
  return name;
#else
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

  const auto mangledName = name.mid(skip).toUtf8();

  int flags = DMGL_PARAMS | DMGL_ANSI | DMGL_VERBOSE;
  auto *res = cplus_demangle(mangledName.constData(), flags);
  if (res == 0) {
    return name;
  }

  auto output = QString::fromUtf8(res);
  free(res);
  return output;
#endif
}

void Util::delayFunc(std::function<void()> func)
{
  QTimer::singleShot(1, func);
}

// Great list: https://sourceforge.net/p/predef/wiki/Architectures/
CpuType Util::currentCpuType()
{
  constexpr bool _64 = (sizeof(void *) == 8);
  (void) _64; // Mark used.

#if defined(i386) || defined(__i386__) || defined(__i386) || defined(__i486__) ||                  \
  defined(__i586__) || defined(__i686__) /* GNU C */ || defined(_M_I86) /* VS */ ||                \
  defined(_M_IX86) /* VS + Intel C + Watcom */ || defined(__X86__) /* Watcom */ ||                 \
  defined(_X86_) /* MinGW32 */
  return CpuType::X86;
#elif defined(__amd64__) || defined(__amd64) || defined(__x86_64__) ||                             \
  defined(__x86_64) /* GNU C + Sun Studio */ || defined(_M_X64) || defined(_M_AMD64) /* VS */
  return CpuType::X86_64;
#elif defined(__arm__) /* GNU C */ || defined(__arm) /* Diab */ || defined(_M_ARM) /* VS */ ||     \
  defined(__TARGET_ARCH_ARM) /* RealView */
  return (_64 ? CpuType::ARM_64 : CpuType::ARM);
#elif defined(__aarch64__)   // GNU C
  return CpuType::ARM_64;
#elif defined(__powerpc) || defined(__powerpc__) || defined(__POWERPC__) || defined(__ppc__) ||    \
  defined(__PPC__) || defined(_ARCH_PPC) /* GNU C */ || defined(_M_PPC) /* VS */
  return CpuType::POWER_PC;
#elif defined(__powerpc64__) || defined(__ppc64__) || defined(__PPC64__) ||                        \
  defined(_ARCH_PPC64)                                   /* GNU C */
  return CpuType::POWER_PC_64;
#elif defined(__sparc__) /* GNU C */ || defined(__sparc) /* Sun Studio */
  return CpuType::SPARC;
#else
  qFatal("Could not detect arch!")
#endif
}

quint64 Util::convertAddress(QString input, bool *ok)
{
  // Simplify.
  input.replace(QRegExp("[\\s\\t\\n\\r]"), "");

  // Try with different bases, starting with auto-detection ("0x" = 16 and "0" = 8).
  for (const auto base : {0, 16, 8, 10}) {
    const auto addr = input.toULongLong(ok, base);
    if (!ok) continue;

    // Reject false positives if input wasn't a zero.
    if (addr == 0 && input != "0") {
      continue;
    }

    return addr;
  }

  if (ok) *ok = false;
  return 0;
}
