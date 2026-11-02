#include "CycleRow.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

#include "qt/theme/CharacterRects.h"
#include "qt/widgets/TitleButton.h"

namespace {
constexpr int kLineSpacing = 8;
constexpr int kBlockSpacing = 0;
}  // namespace

CycleRow::CycleRow(const QString& name, const QPixmap& left, const QPixmap& leftHover,
                   const QPixmap& right, const QPixmap& rightHover, QWidget* parent):
        QWidget(parent), value_(nullptr) {
    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    auto* line = new QHBoxLayout();
    line->setSpacing(kLineSpacing);

    auto* previous = new TitleButton(left, leftHover, name + " anterior", this);
    connect(previous, &QAbstractButton::clicked, this, [this]() { emit stepRequested(-1); });
    line->addWidget(previous);

    auto* block = new QWidget(this);
    auto* blockLayout = new QVBoxLayout(block);
    blockLayout->setContentsMargins(0, 0, 0, 0);
    blockLayout->setSpacing(kBlockSpacing);

    auto* label = new QLabel(name, block);
    label->setAlignment(Qt::AlignHCenter);
    blockLayout->addWidget(label);

    value_ = new QLabel(block);
    value_->setAlignment(Qt::AlignHCenter);
    blockLayout->addWidget(value_);

    line->addWidget(block, 1);

    auto* next = new TitleButton(right, rightHover, name + " siguiente", this);
    connect(next, &QAbstractButton::clicked, this, [this]() { emit stepRequested(1); });
    line->addWidget(next);

    layout->addLayout(line);
}

void CycleRow::setValue(const QString& text) { value_->setText(text); }
