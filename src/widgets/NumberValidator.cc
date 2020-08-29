#include "NumberValidator.h"

namespace dispar {

NumberValidator::NumberValidator(int base_, QObject *parent) : QValidator(parent), base(base_)
{
}

QValidator::State NumberValidator::validate(QString &input, int &pos) const
{
  // Empty string is intermediate, otherwise the string cannot be made empty via backspace or delete
  // actions.
  if (input.isEmpty()) {
    return QValidator::Intermediate;
  }

  bool ok = false;
  input.toULongLong(&ok, base);
  return ok ? QValidator::Acceptable : QValidator::Invalid;
}

} // namespace dispar
