#include "InputBox.h"

#include <QLineEdit>
#include <QVBoxLayout>

#include "qt/theme/GameCursor.h"

InputBox::InputBox(const QPixmap& box, const QMargins& padding, QWidget* parent):
        QLabel(parent), edit_(new QLineEdit(this)) {
    setPixmap(box);
    setFixedSize(box.size());

    edit_->setFrame(false);
    edit_->setCursor(GameCursor().cursor());

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(padding);
    layout->addWidget(edit_);
}

QLineEdit* InputBox::edit() const { return edit_; }
