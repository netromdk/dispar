#include "widgets/TagsEdit.h"
#include "Context.h"
#include "Project.h"
#include "widgets/TagItemDelegate.h"

#include <QDebug>
#include <QEvent>
#include <QKeyEvent>
#include <QLineEdit>
#include <QListWidget>
#include <QMessageBox>
#include <QVBoxLayout>

TagsEdit::TagsEdit() : address(0)
{
  itemDelegate = new TagItemDelegate;
  createLayout();
}

TagsEdit::~TagsEdit()
{
  itemDelegate->deleteLater();
}

void TagsEdit::setAddress(quint64 address)
{
  this->address = address;

  auto project = Context::get().project();
  connect(project.get(), &Project::tagsChanged, this, &TagsEdit::updateTags);
  listWidget->setEnabled(true);
  updateTags();

  lineEdit->clear();
  lineEdit->setEnabled(true);
}

bool TagsEdit::eventFilter(QObject *obj, QEvent *event)
{
  if (obj == listWidget) {
    if (event->type() == QEvent::KeyPress) {
      auto *keyEvent = static_cast<QKeyEvent *>(event);
      switch (keyEvent->key()) {
      case Qt::Key_Backspace:
      case Qt::Key_Delete:
        removeTag();
        break;
      }
      return true;
    }
  }

  return QObject::eventFilter(obj, event);
}

void TagsEdit::onReturnPressed()
{
  auto tag = lineEdit->text().simplified();
  if (tag.isEmpty()) return;

  auto project = Context::get().project();
  if (!project->addAddressTag(tag, address)) {
    QMessageBox::warning(this, "dispar", tr("Tag is already used!"));
    return;
  }

  lineEdit->clear();
}

void TagsEdit::updateTags()
{
  listWidget->clear();
  listWidget->addItems(Context::get().project()->addressTags(address));
}

void TagsEdit::createLayout()
{
  listWidget = new QListWidget;
  listWidget->setEnabled(false);
  listWidget->setViewMode(QListView::IconMode);
  listWidget->setItemDelegate(itemDelegate);
  listWidget->installEventFilter(this);

  lineEdit = new QLineEdit;
  lineEdit->setEnabled(false);
  lineEdit->setPlaceholderText(tr("Input tag and press enter.."));
  connect(lineEdit, &QLineEdit::returnPressed, this, &TagsEdit::onReturnPressed);

  auto *layout = new QVBoxLayout;
  layout->setContentsMargins(0, 0, 0, 0);
  layout->setSpacing(1);
  layout->addWidget(listWidget);
  layout->addWidget(lineEdit);

  setLayout(layout);
}

void TagsEdit::removeTag()
{
  auto *item = listWidget->currentItem();
  if (!item) return;

  auto tag = item->text();
  Context::get().project()->removeAddressTag(tag);

  delete listWidget->takeItem(listWidget->row(item));
}
