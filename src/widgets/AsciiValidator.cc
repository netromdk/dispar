#include "AsciiValidator.h"

#include <cctype>

namespace dispar {

AsciiValidator::AsciiValidator(QObject *parent) : QValidator(parent)
{
}

QValidator::State AsciiValidator::validate(QString &input, int &pos) const
{
  // Empty string is intermediate, otherwise the string cannot be made empty via backspace or delete
  // actions.
  if (input.isEmpty()) {
    return QValidator::Intermediate;
  }

  bool ok = true;
  for (auto &&i : input) {
    const auto ch = uint(i.toLatin1());
    ok &= std::isprint(ch) > 0;
    if (!ok) break;
  }
  return ok ? QValidator::Acceptable : QValidator::Invalid;
}

} // namespace dispar
