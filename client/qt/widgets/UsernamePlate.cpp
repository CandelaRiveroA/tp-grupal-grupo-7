#include "UsernamePlate.h"

#include <QFontMetrics>
#include <QPainter>

#include <utility>

#include "qt/theme/TitleSpriteRects.h"

UsernamePlate::UsernamePlate(const QPixmap& plate, QWidget* parent): QWidget(parent), plate_(plate) {
    setObjectName("usernamePlate");
    setFixedSize(TITLE_PLATE_W, TITLE_PLATE_H);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setVisible(false);
}

void UsernamePlate::setUsername(std::string username) {
    username_ = std::move(username);
    setVisible(!username_.empty());
    update();
}

void UsernamePlate::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, false);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);

    painter.drawPixmap(0, 0, plate_);

    if (username_.empty()) {
        return;
    }

    const QRect textRect(TITLE_PLATE_TEXT_INSET_X, TITLE_PLATE_TEXT_INSET_TOP,
                         width() - 2 * TITLE_PLATE_TEXT_INSET_X,
                         height() - TITLE_PLATE_TEXT_INSET_TOP - TITLE_PLATE_TEXT_INSET_BOTTOM);

    QFont font(TITLE_PLATE_FONT_FAMILY);
    font.setPixelSize(TITLE_PLATE_FONT_PX);
    painter.setFont(font);
    painter.setPen(QColor(TITLE_PLATE_TEXT_COLOR));

    const QFontMetrics metrics(font);
    const QString text = metrics.elidedText(QString::fromStdString(username_), Qt::ElideRight,
                                            textRect.width());
    painter.drawText(textRect, Qt::AlignCenter, text);
}
