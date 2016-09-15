#ifndef DISPAR_READER_H
#define DISPAR_READER_H

#include <QByteArray>

class QIODevice;

class Reader {
public:
  Reader(QIODevice &dev, bool littleEndian = true);

  bool isLittleEndian() const;
  void setLittleEndian(bool little);

  quint16 getUInt16(bool *ok = nullptr);
  quint32 getUInt32(bool *ok = nullptr);
  quint64 getUInt64(bool *ok = nullptr);

  char getChar(bool *ok = nullptr);
  unsigned char getUChar(bool *ok = nullptr);
  char peekChar(bool *ok = nullptr);
  unsigned char peekUChar(bool *ok = nullptr);

  QByteArray read(qint64 max);

  qint64 pos() const;
  bool seek(qint64 pos);
  bool atEnd() const;

  bool peekList(std::initializer_list<unsigned char> list);

private:
  template <typename T>
  T getUInt(bool *ok = nullptr);

  QIODevice &dev;
  bool littleEndian;
};

#endif // DISPAR_READER_H
