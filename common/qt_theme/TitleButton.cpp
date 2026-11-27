#include "TitleButton.h"

#include <QEnterEvent>
#include <QPainter>

TitleButton::TitleButton(const QPixmap& normal, const QPixmap& hover, const QString& accessibleText,
                         QWidget* parent):
        QAbstractButton(parent), normal_(normal), hover_(hover) {
    setAccessibleName(accessibleText);
    setToolTip(accessibleText);
    setFocusPolicy(Qt::StrongFocus);
    setAttribute(Qt::WA_Hover);
}

QSize TitleButton::sizeHint() const { return normal_.size(); }

void TitleButton::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    if (!isEnabled()) {
        painter.setOpacity(0.5);
        painter.drawPixmap(0, 0, normal_);
        return;
    }
    if (underMouse() || hasFocus()) {
        painter.drawPixmap(0, 0, hover_);
        return;
    }
    painter.drawPixmap(0, 0, normal_);
}

void TitleButton::enterEvent(QEnterEvent* event) {
    QAbstractButton::enterEvent(event);
    update();
}

void TitleButton::leaveEvent(QEvent* event) {
    QAbstractButton::leaveEvent(event);
    update();
}
