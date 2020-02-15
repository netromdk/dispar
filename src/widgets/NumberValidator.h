#ifndef SRC_WIDGETS_NUMBERVALIDATOR_H
#define SRC_WIDGETS_NUMBERVALIDATOR_H

#include <QValidator>

namespace dispar {

class NumberValidator : public QValidator {
public:
  NumberValidator(int base, QObject *parent = nullptr);

  QValidator::State validate(QString &input, int &pos) const override;

private:
  int base;
};

} // namespace dispar

#endif // SRC_WIDGETS_NUMBERVALIDATOR_H
