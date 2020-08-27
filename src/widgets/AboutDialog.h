#ifndef DISPAR_ABOUT_DIALOG_H
#define DISPAR_ABOUT_DIALOG_H

#include <QDialog>

namespace dispar {

class AboutDialog : public QDialog {
public:
  AboutDialog(QWidget *parent = nullptr);
  ~AboutDialog() override;

private:
  void createLayout();
  void addLinkLabel(const QString &url, const QString &text, const QString &pretext = {});
  void addDep(const QString &name, const QString &version, const QString &url);

  int yPos = 0;
};

} // namespace dispar

#endif // DISPAR_ABOUT_DIALOG_H
