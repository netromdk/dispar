#include "Util.h"
#include "BinaryObject.h"
#include "formats/Format.h"

#include <QAbstractScrollArea>
#include <QApplication>
#include <QDebug>
#include <QDesktopWidget>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>
#include <QScreen>
#include <QScrollBar>
#include <QStringList>
#include <QTimer>
#include <QTreeWidgetItem>
#include <QXmlStreamReader>

#include <cctype>
#include <cmath>
#include <utility>

#include "libiberty/demangle.h"

namespace dispar {

QString Util::formatSize(qint64 bytes, int digits)
{
  QString unit{"B"};
  double size = bytes;

  static const QStringList factors{"KB", "MB", "GB", "TB", "PB", "EB"};
  for (int i = factors.size() - 1; i >= 0; --i) {
    if (const auto boundary = std::pow(1024.0, double(i + 1)); size >= boundary) {
      size /= boundary;
      unit = factors[i];
      break;
    }
  }

  return QString("%1 %2").arg(QString::number(size, 'f', digits)).arg(unit);
}

QString Util::padString(const QString &str, int size, bool before, char pad)
{
  int addrLen = str.size();
  if (addrLen < size) {
    const QString padStr(size - addrLen, pad);

    // cppcheck: No, they are not the same in both branches!
    return (before ? padStr + str : str + padStr);
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

QString Util::hexToAscii(const QString &str, int offset, int blocks, int blocksize, bool unicode)
{
  QString res;
  int len = str.size();
  int size = blocks * blocksize;
  bool ok = false;
  for (int i = offset; i - offset <= size && i < len; i += blocksize) {
    if (str[i] == ' ') {
      size++;
      i--;
      continue;
    }
    int ic = str.mid(i, blocksize).toInt(&ok, 16);
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

QString Util::hexToUnicode(const QString &str)
{
  QString res;

  // Split on whitespace and treat each part separately.
  for (const auto &part : str.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts)) {
    int blocksize = part.size();
    if (blocksize < 2 || blocksize > 4) {
      blocksize = 2;
    }
    res += hexToAscii(part, 0, part.size() / blocksize, blocksize, true);
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
  bool ok = false;
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
    return path;
  }

  QDir dir(path);
  if (!dir.exists() || !dir.cd("Contents")) {
    return path;
  }

  const auto plistFile = dir.absoluteFilePath("Info.plist");

  if (!dir.cd("MacOS")) {
    return path;
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
    return path;
  }

  QFile file(plistFile);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    return path;
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
        return path;
      }
      return dir.absoluteFilePath(execName);
    }
  }

  return path;
}

QScreen *Util::screenOfWidget(QWidget *widget)
{
  const auto number = QApplication::desktop()->screenNumber(widget);
  if (number == -1) return nullptr;
  return QGuiApplication::screens()[number];
}

void Util::centerWidget(QWidget *widget)
{
  if (const auto *screen = screenOfWidget(widget); screen) {
    widget->move(screen->availableGeometry().center() - widget->rect().center());
  }
}

void Util::resizeRatioOfScreen(QWidget *widget, float percentage, const QSize &minimum)
{
  const auto *screen = screenOfWidget(widget);
  if (screen == nullptr) return;

  auto rect = screen->availableGeometry();
  rect.setWidth(int(float(rect.width()) * percentage));
  rect.setHeight(int(float(rect.height()) * percentage));

  auto size = rect.size();
  if (size.width() < minimum.width()) {
    size.setWidth(minimum.width());
  }
  if (size.height() < minimum.height()) {
    size.setHeight(minimum.height());
  }

  widget->resize(size);
}

QString Util::addrDataString(quint64 addr, QByteArray data)
{
  QString output, ascii;
  QStringList lines;

  const auto addLine = [&](const QString &line) {
    lines.append(QString::number(addr, 16).toUpper() + ": " + line);
  };

  for (int i = 0, j = 0; i < data.size(); ++i) {
    char c = data[i];
    int ic = c;
    unsigned char uc = c;
    QString hc = QString::number(uc, 16).toUpper();
    if (hc.size() == 1) {
      hc = "0" + hc;
    }
    output += hc + " ";

    // Put an extra space in the middle of the hex data to show the low and high parts.
    if (j++ == 7) {
      output += " ";
    }

    ascii += (std::isgraph(ic) > 0 ? c : '.');
    if ((i + 1) % 16 == 0 || i == data.size() - 1) {
      // If hex amount is < 16 blocks (including extra spaces) then pad with spaces, not zeros.
      if (const auto n = output.size(), max = 16 * 2 + 16 + 1; n < max) {
        output += QString(max - n, ' ');
      }

      output += "  " + ascii;
      ascii.clear();
      if (i != data.size() - 1) {
        addLine(output);
        addr += 16;
        output.clear();
        j = 0;
      }
    }
  }

  // Add the last line.
  addLine(output);

  return lines.join("\n");
}

void Util::scrollToTop(QAbstractScrollArea *widget)
{
  QTimer::singleShot(1, widget, [widget] {
    auto *bar = widget->verticalScrollBar();
    if (bar != nullptr) {
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
  const int flags = DMGL_PARAMS | DMGL_ANSI | DMGL_VERBOSE;

  // Demangled char* result must be freed instead of deleted!
  const auto deleter = [](char *c) {
    free(c); // NOLINT
  };

  if (std::unique_ptr<char, decltype(deleter)> demangled(
        cplus_demangle(mangledName.constData(), flags), deleter);
      demangled != nullptr) {
    return QString::fromUtf8(demangled.get());
  }

  return name;
#endif
}

void Util::delayFunc(std::function<void()> func)
{
  QTimer::singleShot(1, std::move(func));
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
  input.replace(QRegExp(R"([\s\t\n\r])"), "");

  // Try with different bases, starting with auto-detection ("0x" = 16 and "0" = 8).
  for (const auto base : {0, 16, 8, 10}) {
    const auto addr = input.toULongLong(ok, base);
    if (ok != nullptr && !*ok) {
      continue;
    }

    // Reject false positives if input wasn't a zero.
    if (addr == 0 && input != "0") {
      continue;
    }

    return addr;
  }

  if (ok != nullptr) *ok = false;
  return 0;
}

void Util::setTreeItemMarked(QTreeWidgetItem *item, int column)
{
  auto font = item->font(column);
  font.setBold(true);
  item->setFont(column, font);
  item->setForeground(column, Qt::red);
}

QString Util::byteArrayString(const QByteArray &array)
{
  return QString::fromLatin1(array.toHex());
}

QByteArray Util::byteArray(const QString &value)
{
  return QByteArray::fromHex(value.toLatin1());
}

QString Util::escapeWhitespace(QString value)
{
  return value.replace("\n", "\\n")
    .replace("\t", "\\t")
    .replace("\r", "\\r")
    .replace("\v", "\\v")
    .replace("\f", "\\f");
}

std::tuple<int, int> Util::decodeMacSdkVersion(const quint32 version)
{
  return {version >> 16, (version & 0x00FFF) >> 8};
}

quint32 Util::encodeMacSdkVersion(const std::tuple<int, int> &version)
{
  return std::get<0>(version) << 16 | std::get<1>(version) << 8;
}

QByteArray Util::longToData(const unsigned long n)
{
  // Union is used to convert from unsigned long to char[4], and a std::variant cannot achieve that
  // wrt. cppcoreguidelines-pro-type-union-access.
  union {
    std::array<char, 4> chars;
    unsigned long n;
  } ci{};
  ci.n = n;                     // NOLINT(cppcoreguidelines-pro-type-union-access)
  const auto &chars = ci.chars; // NOLINT(cppcoreguidelines-pro-type-union-access)

  QByteArray data;
  for (char i : chars) {
    data.append(i);
  }
  return data;
}

} // namespace dispar
