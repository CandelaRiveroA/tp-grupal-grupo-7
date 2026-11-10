#include "WorldButton.h"

#include <QEnterEvent>
#include <QEvent>
#include <QPaintEvent>
#include <QPainter>

#include "qt/theme/WorldSpriteRects.h"
#include "qt/theme/WorldSprites.h"

using namespace WorldSpriteRects;

WorldButton::WorldButton(const WorldSummary& world, const WorldSprites& sprites, QWidget* parent):
        QAbstractButton(parent), world_(world), sprites_(sprites) {
    setObjectName("worldButton");
    setCheckable(true);
    setAttribute(Qt::WA_Hover);
    setFixedSize(kWorldButtonWidth, kWorldButtonHeight);
    setFocusPolicy(Qt::StrongFocus);
    setAccessibleName(QString::fromStdString(world_.name));
    if (!world_.description.empty()) {
        setAccessibleDescription(QString::fromStdString(world_.description));
    }
    setToolTip(makeToolTip());
}

const WorldSummary& WorldButton::world() const { return world_; }

void WorldButton::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    if (!isEnabled()) {
        painter.setOpacity(kDisabledOpacity);
    }
    if (underMouse() || hasFocus()) {
        painter.drawPixmap(kIconOffsetX, kIconOffsetY, sprites_.hoverIconFor(world_.id));
    } else {
        painter.drawPixmap(kIconOffsetX, kIconOffsetY, sprites_.iconFor(world_.id));
    }
    if (isChecked()) {
        painter.drawPixmap(kIconOffsetX, kIconOffsetY, sprites_.selectedBorder());
    }
}

void WorldButton::enterEvent(QEnterEvent* event) {
    QAbstractButton::enterEvent(event);
    update();
}

void WorldButton::leaveEvent(QEvent* event) {
    QAbstractButton::leaveEvent(event);
    update();
}

QString WorldButton::makeToolTip() const {
    const QString name = QString::fromStdString(world_.name).toHtmlEscaped();
    const QString description =
            QString::fromStdString(world_.description).trimmed().toHtmlEscaped();
    if (description.isEmpty()) {
        return QString("<b>%1</b>").arg(name);
    }
    return QString("<b>%1</b><br/>%2").arg(name, description);
}
