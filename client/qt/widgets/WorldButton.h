#pragma once

#include <QAbstractButton>
#include <QString>

#include "common/model/draft/WorldSummary.h"

class QEnterEvent;
class QEvent;
class QPaintEvent;
class WorldSprites;

class WorldButton: public QAbstractButton {
    Q_OBJECT

public:
    WorldButton(const WorldSummary& world, const WorldSprites& sprites, QWidget* parent = nullptr);

    const WorldSummary& world() const;

protected:
    void paintEvent(QPaintEvent* event) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    QString makeToolTip() const;

    WorldSummary world_;
    const WorldSprites& sprites_;
};
