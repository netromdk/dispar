#ifndef DISPAR_UTIL_H
#define DISPAR_UTIL_H

#include <QByteArray>
#include <QString>
#include <QWidget>

class QAbstractScrollArea;

class Util {
public:
  static QString formatSize(qint64 bytes, int digits = 1);

  // char(48) = '0'
  static QString padString(const QString &str, int size, bool before = true, char pad = 48);

  static QString dataToAscii(const QByteArray &data, int offset, int size);
  static QString hexToAscii(const QString &data, int offset, int blocks, bool unicode = false);
  static QString hexToString(const QString &str);
  static QByteArray hexToData(const QString &str);

  static QString resolveAppBinary(QString path);

  static void centerWidget(QWidget *widget);

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
};

#endif // DISPAR_UTIL_H
