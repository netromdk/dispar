#ifndef SRC_WIDGETS_ASCIIVALIDATOR_H
#define SRC_WIDGETS_ASCIIVALIDATOR_H

#include <QValidator>

namespace dispar {

class AsciiValidator : public QValidator {
public:
  AsciiValidator(QObject *parent = nullptr);

  QValidator::State validate(QString &input, int &pos) const override;
};

} // namespace dispar

#endif // SRC_WIDGETS_ASCIIVALIDATOR_H
