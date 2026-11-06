#include "ParchmentBanner.h"

#include <QLabel>
#include <QPaintEvent>
#include <QPainter>
#include <QVBoxLayout>

ParchmentBanner::ParchmentBanner(const QPixmap& banner, QLabel* label, QWidget* parent):
        QWidget(parent), banner_(banner) {
    setFixedSize(banner_.size());

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(8, 2, 8, 2);
    if (label != nullptr) {
        label->setParent(this);
        label->setAlignment(Qt::AlignCenter);
        layout->addWidget(label);
    }
}

void ParchmentBanner::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    painter.drawPixmap(0, 0, banner_);
    QWidget::paintEvent(event);
}
