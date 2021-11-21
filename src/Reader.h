#ifndef DISPAR_READER_H
#define DISPAR_READER_H

#include "Constants.h"
#include <QtGlobal>

class QIODevice;
class QByteArray;

namespace dispar {

class Reader {
public:
  Reader(QIODevice &dev, Constants::Endianness endianness = Constants::Endianness::Little);

  [[nodiscard]] Constants::Endianness endianness() const;

  void setEndianness(Constants::Endianness);

  /// Read 2 bytes = 16 bits.
  quint16 getUInt16(bool *ok = nullptr);

  /// Read 4 bytes = 32 bits.
  quint32 getUInt32(bool *ok = nullptr);

  /// Read 8 bytes = 64 bits.
  quint64 getUInt64(bool *ok = nullptr);

  char getChar(bool *ok = nullptr);
  unsigned char getUChar(bool *ok = nullptr);
  char peekChar(bool *ok = nullptr);
  unsigned char peekUChar(bool *ok = nullptr);

  QByteArray read(qint64 max);

  [[nodiscard]] qint64 pos() const;
  bool seek(qint64 pos);
  [[nodiscard]] bool atEnd() const;

  bool peekList(std::initializer_list<unsigned char> list);

private:
  template <typename T>
  T getUInt(bool *ok = nullptr);

  QIODevice &dev;
  Constants::Endianness endianness_;
};

} // namespace dispar

#endif // DISPAR_READER_H
