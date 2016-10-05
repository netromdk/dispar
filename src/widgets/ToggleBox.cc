#include "ToggleBox.h"

#include <QScrollArea>
#include <QToolButton>
#include <QVBoxLayout>

ToggleBox::ToggleBox(const QString &title, QWidget *parent) : QWidget(parent), title(title)
{
  createLayout();
  setCollapsed();
}

void ToggleBox::setContentLayout(QLayout *layout)
{
  Q_ASSERT(layout);
  if (contentWidget->layout()) {
    delete contentWidget->layout();
  }

  contentWidget->setLayout(layout);

  // Set new maximum height to match tool button height and content layout height.
  auto collapsedHeight = sizeHint().height() - contentWidget->maximumHeight();
  auto contentHeight = layout->sizeHint().height();
  setMaximumHeight(collapsedHeight + contentHeight);
}

void ToggleBox::setCollapsed(bool collapsed)
{
  toggleButton->setArrowType(collapsed ? Qt::ArrowType::RightArrow : Qt::ArrowType::DownArrow);
  toggleButton->setChecked(!collapsed);

  if (collapsed) {
    contentWidget->setMinimumHeight(0);
    contentWidget->setMaximumHeight(0);
  }
  else {
    contentWidget->setMinimumHeight(minimumHeight());
    contentWidget->setMaximumHeight(maximumHeight());
  }
}

void ToggleBox::setExpanded(bool expanded)
{
  setCollapsed(!expanded);
}

void ToggleBox::createLayout()
{
  toggleButton = new QToolButton;
  toggleButton->setStyleSheet("QToolButton { border: none; }");
  toggleButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
  toggleButton->setText(title);
  toggleButton->setCheckable(true);
  toggleButton->setChecked(false);

  connect(toggleButton, &QToolButton::clicked, this,
          [this](bool checked) { setCollapsed(!checked); });

  contentWidget = new QScrollArea;
  contentWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  // Draw rounded edges on the content widget using a 5% darker window background color.
  auto winColor = contentWidget->palette().window().color().darker(105);
  contentWidget->setStyleSheet(
    QString("QScrollArea { background: %1; border: 1px solid %1; border-radius: 5px; }")
      .arg(winColor.name()));

  auto *layout = new QVBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(0);
  layout->addWidget(toggleButton);
  layout->addWidget(contentWidget);

  setLayout(layout);
}
