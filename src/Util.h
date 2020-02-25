#ifndef DISPAR_UTIL_H
#define DISPAR_UTIL_H

#include <QByteArray>
#include <QString>
#include <QWidget>

#include <algorithm>
#include <functional>
#include <iterator>
#include <tuple>

#include "CpuType.h"
#include "cxx.h"

class QIODevice;
class QScreen;
class QTreeWidgetItem;
class QAbstractScrollArea;

namespace dispar {

class Format;

class Util {
public:
  static QString formatSize(qint64 bytes, int digits = 1);

  // char(48) = '0'
  static QString padString(const QString &str, int size, bool before = true, char pad = 48);

  static QString dataToAscii(const QByteArray &data, int offset, int size);
  static QString hexToAscii(const QString &data, int offset, int blocks, int blocksize = 2,
                            bool unicode = false);
  static QString hexToUnicode(const QString &data);
  static QString hexToString(const QString &str);
  static QByteArray hexToData(const QString &str);

  static QString bytesToHex(const unsigned char *bytes, int size);

  static QString resolveAppBinary(QString path);

  static QScreen *screenOfWidget(QWidget *widget);

  static void centerWidget(QWidget *widget);

  /// Resize \p widget to \p percentage of the default screen.
  /** Percentage is ]0, 1]. */
  static void resizeRatioOfScreen(QWidget *widget, float percentage,
                                  const QSize &minimum = QSize(800, 600));

  /**
   * Generate string of format:
   *
   * ADDR: 00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  . . . . . . . . . . . . . . . .
   * ADDR: 02 00 01 04 03 06 07 05 08 09 0A 0D 0B 0C 0F 0E  . . . . . . . . . . . . . . . .
   *
   * And so on, where ADDR is the address followed by data in hex and
   * ASCII representation of meaningful values.
   */
  static QString addrDataString(quint64 addr, QByteArray data);

  /// Scroll area to top.
  static void scrollToTop(QAbstractScrollArea *widget);

  /// Demangle ABI identifier.
  static QString demangle(const QString &name);

  static void delayFunc(std::function<void()> func);

  static CpuType currentCpuType();

  /// Try to convert \p input to an address and write status to \p ok, if specified.
  /** It will remove all white space from \p input and try to convert using base 16 if it starts
      with "0x", base 8 if it starts with "0", and otherwise base 10. */
  static quint64 convertAddress(QString input, bool *ok = nullptr);

  /// Moves values from \p src to \p dst but preserves any values in \p dst.
  template <typename Container>
  static void moveTo(Container &src, Container &dst)
  {
    if (dst.empty()) {
      dst = std::move(src);
      return;
    }

    dst.reserve(dst.size() + src.size());
    cxx::move(src, std::back_inserter(dst));
    src.clear();
  }

  /// Append values from \p src to \p dst.
  template <typename Container>
  static void copyTo(const Container &src, Container &dst)
  {
    dst.reserve(dst.size() + src.size());
    cxx::copy(src, std::back_inserter(dst));
  }

  /// Set item as marked by using bold font and red text on \p column.
  static void setTreeItemMarked(QTreeWidgetItem *item, int column);

  /// Convert byte \p array safely into hex-encoded string.
  static QString byteArrayString(const QByteArray &array);

  /// Convert hex-encoded \p value into byte array.
  static QByteArray byteArray(const QString &value);

  /// Escapes all whitespace in \p value.
  /** So "\n" will be "\\n", for instance. */
  static QString escapeWhitespace(QString value);

  /// Decode version from format 0xXXYYY as x.y.
  static std::tuple<int, int> decodeMacSdkVersion(const quint32 version);

  /// Encode version x.y as 0xXXYYY.
  static quint32 encodeMacSdkVersion(const std::tuple<int, int> &version);

  static QByteArray longToData(const unsigned long n);
};

} // namespace dispar

#endif // DISPAR_UTIL_H
