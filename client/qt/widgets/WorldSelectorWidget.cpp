#include "WorldSelectorWidget.h"

#include <QAbstractButton>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLayoutItem>
#include <QPaintEvent>
#include <QPainter>
#include <QVBoxLayout>
#include <algorithm>
#include <cstddef>
#include <vector>

#include "qt/theme/WorldSpriteRects.h"
#include "qt/widgets/WorldButton.h"

WorldSelectorWidget::WorldSelectorWidget(QWidget* parent):
        QWidget(parent), columns_(new QHBoxLayout(this)), group_(new QButtonGroup(this)) {
    setObjectName("worldSelector");
    columns_->setContentsMargins(0, 0, 0, 0);
    columns_->setSpacing(WorldSpriteRects::kColumnGap);
    group_->setExclusive(true);
    connect(group_, &QButtonGroup::idClicked, this, [this](int id) {
        if (auto* button = qobject_cast<WorldButton*>(group_->button(id))) {
            emit worldSelected(button->world());
        }
    });
}

void WorldSelectorWidget::paintEvent(QPaintEvent*) {
    if (panel_.size() != size()) {
        panel_ = sprites_.panelFrame(width(), height());
    }
    QPainter painter(this);
    painter.drawPixmap(0, 0, panel_);
}

void WorldSelectorWidget::setWorlds(const std::vector<WorldSummary>& worlds) {
    clearButtons();

    const int count = std::min(static_cast<int>(worlds.size()), WorldSpriteRects::kMaxWorlds);
    QWidget* column = nullptr;
    QVBoxLayout* columnLayout = nullptr;
    for (int i = 0; i < count; ++i) {
        if (i % WorldSpriteRects::kMaxRowsPerColumn == 0) {
            if (columnLayout != nullptr) {
                columnLayout->addStretch();
            }
            column = makeColumn();
            columnLayout = qobject_cast<QVBoxLayout*>(column->layout());
        }
        auto* button = new WorldButton(worlds[static_cast<std::size_t>(i)], sprites_, column);
        group_->addButton(button, i);
        columnLayout->addWidget(button, 0, Qt::AlignHCenter);
    }
    if (columnLayout != nullptr) {
        columnLayout->addStretch();
    }

    // Seleccion inicial
    if (auto* first = group_->button(0)) {
        first->setChecked(true);
    }
}

std::optional<WorldSummary> WorldSelectorWidget::selectedWorld() const {
    auto* button = qobject_cast<WorldButton*>(group_->checkedButton());
    if (button == nullptr) {
        return std::nullopt;
    }
    return button->world();
}

void WorldSelectorWidget::clearButtons() {
    //cada columna es un widgety borrarla destruye sus botones y su layout.
    while (QLayoutItem* item = columns_->takeAt(0)) {
        delete item->widget();
        delete item;
    }
}

QWidget* WorldSelectorWidget::makeColumn() {
    auto* column = new QWidget(this);
    auto* layout = new QVBoxLayout(column);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(WorldSpriteRects::kRowGap);
    columns_->addWidget(column, 0, Qt::AlignTop);
    return column;
}
