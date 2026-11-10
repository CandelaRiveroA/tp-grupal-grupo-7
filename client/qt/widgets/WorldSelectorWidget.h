#pragma once

#include <QPixmap>
#include <QWidget>
#include <optional>
#include <vector>

#include "common/model/draft/WorldSummary.h"
#include "qt/theme/WorldSprites.h"

class QButtonGroup;
class QHBoxLayout;
class QPaintEvent;

class WorldSelectorWidget: public QWidget {
    Q_OBJECT

public:
    explicit WorldSelectorWidget(QWidget* parent = nullptr);

    void setWorlds(const std::vector<WorldSummary>& worlds);
    std::optional<WorldSummary> selectedWorld() const;

signals:
    void worldSelected(WorldSummary world);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    void clearButtons();
    QWidget* makeColumn();

    WorldSprites sprites_;
    QPixmap panel_;
    QHBoxLayout* columns_;
    QButtonGroup* group_;
};
