#include "gtest/gtest.h"

#include "testutils.h"

#include <QEventLoop>

#include "AddrHexAsciiEncoder.h"
#include "Section.h"
using namespace dispar;

TEST(AddrHexAsciiEncoder, async)
{
  const quint64 addr(0x1000);
  const QByteArray data(3, 'x');
  const quint64 size = data.size();
  Section section(Section::Type::TEXT, "Test", addr, size);
  section.setData(data);

  AddrHexAsciiEncoder encoder(&section);

  QEventLoop loop;
  QObject::connect(&encoder, &AddrHexAsciiEncoder::finished, &loop, &QEventLoop::quit);

  encoder.start();
  loop.exec();

  EXPECT_EQ(encoder.result(),
            "1000: 78 78 78 00 00 00 00 00 00 00 00 00 00 00 00 00   xxx.............")
    << encoder.result();
}

TEST(AddrHexAsciiEncoder, blocking)
{
  const quint64 addr(0x1000);
  const QByteArray data(3, 'x');
  const quint64 size = data.size();
  Section section(Section::Type::TEXT, "Test", addr, size);
  section.setData(data);

  AddrHexAsciiEncoder encoder(&section);

  const bool blocking(true);
  encoder.start(blocking);

  EXPECT_EQ(encoder.result(),
            "1000: 78 78 78 00 00 00 00 00 00 00 00 00 00 00 00 00   xxx.............")
    << encoder.result();
}
