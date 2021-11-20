#include "Reader.h"
#include "Constants.h"

#include <QByteArray>
#include <QIODevice>

namespace dispar {

Reader::Reader(QIODevice &dev_, Constants::Endianness endianness_)
  : dev(dev_), endianness_(endianness_)
{
}

Constants::Endianness Reader::endianness() const
{
  return endianness_;
}

void Reader::setEndianness(Constants::Endianness endianness)
{
  endianness_ = endianness;
}

quint16 Reader::getUInt16(bool *ok)
{
  return getUInt<quint16>(ok);
}

quint32 Reader::getUInt32(bool *ok)
{
  return getUInt<quint32>(ok);
}

quint64 Reader::getUInt64(bool *ok)
{
  return getUInt<quint64>(ok);
}

char Reader::getChar(bool *ok)
{
  char c{0};
  bool res = dev.getChar(&c);
  if (ok != nullptr) *ok = res;
  return c;
}

unsigned char Reader::getUChar(bool *ok)
{
  return static_cast<unsigned char>(getChar(ok));
}

char Reader::peekChar(bool *ok)
{
  char c{0};
  qint64 num = dev.peek(&c, 1);
  if (ok != nullptr) *ok = (num == 1);
  return c;
}

unsigned char Reader::peekUChar(bool *ok)
{
  return static_cast<unsigned char>(peekChar(ok));
}

QByteArray Reader::read(qint64 max)
{
  return dev.read(max);
}

qint64 Reader::pos() const
{
  return dev.pos();
}

bool Reader::seek(qint64 pos)
{
  return dev.seek(pos);
}

bool Reader::atEnd() const
{
  return dev.atEnd();
}

bool Reader::peekList(std::initializer_list<unsigned char> list)
{
  if (list.size() == 0) {
    return false;
  }

  const auto parr = dev.peek(list.size());
  if (parr.size() != static_cast<qint64>(list.size())) {
    return false;
  }

  int i = 0;
  for (const auto &c : list) {
    if (c != static_cast<unsigned char>(parr[i])) {
      return false;
    }
    i++;
  }
  return true;
}

template <typename T>
T Reader::getUInt(bool *ok)
{
  constexpr int num = sizeof(T);
  QByteArray buf = dev.read(num);
  if (buf.size() < num) {
    if (ok) *ok = false;
    return 0;
  }
  T res{0};
  for (int i = 0; i < num; i++) {
    int j = i;
    if (endianness_ == Constants::Endianness::Big) {
      j = num - (i + 1);
    }
    res += ((T) static_cast<unsigned char>(buf[i])) << j * 8;
  }
  if (ok) *ok = true;
  return res;
}

} // namespace dispar
