#include "SliderRow.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSignalBlocker>
#include <QSlider>
#include <QVBoxLayout>

#include "qt/theme/GameCursor.h"

namespace {
constexpr int kBlockSpacing = 2;
constexpr int kLineSpacing = 12;
constexpr int kValueMinWidth = 24;
}  // namespace

SliderRow::SliderRow(const QString& name, const QString& objectName, int maximum, QWidget* parent):
        QWidget(parent), slider_(nullptr), value_(nullptr) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(kBlockSpacing);

    layout->addWidget(new QLabel(name, this));

    auto* line = new QHBoxLayout();
    line->setSpacing(kLineSpacing);

    slider_ = new QSlider(Qt::Horizontal, this);
    slider_->setObjectName(objectName);
    slider_->setCursor(GameCursor().cursor());
    slider_->setRange(0, maximum);
    slider_->setSingleStep(1);
    slider_->setPageStep(1);
    connect(slider_, &QSlider::valueChanged, this, &SliderRow::valueChanged);
    line->addWidget(slider_, 1);

    value_ = new QLabel(this);
    value_->setMinimumWidth(kValueMinWidth);
    value_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    line->addWidget(value_);

    layout->addLayout(line);
}

void SliderRow::setValue(int value) {
    const QSignalBlocker blocker(slider_);
    slider_->setValue(value);
    value_->setText(QString::number(value));
}
