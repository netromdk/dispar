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

  const auto ch = uint(input[0].toLatin1());
  return std::isprint(ch) ? QValidator::Acceptable : QValidator::Invalid;
}

} // namespace dispar
