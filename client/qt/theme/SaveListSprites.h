#pragma once

#include <QPixmap>
#include <QtGlobal>

class SaveListSprites {
public:
    SaveListSprites();

    const QPixmap& windowBox() const;
    const QPixmap& rowBox() const;
    const QPixmap& rowBoxHover() const;
    const QPixmap& upArrow() const;
    const QPixmap& upArrowHover() const;
    const QPixmap& downArrow() const;
    const QPixmap& downArrowHover() const;
    const QPixmap& deleteIcon() const;
    const QPixmap& deleteIconHover() const;
    const QPixmap& coinIcon() const;
    const QPixmap& clockIcon() const;
    const QPixmap& coopBadge() const;

private:
    QPixmap load(const char* path) const;
    QPixmap brightenPercent(const QPixmap& sprite) const;
    QPixmap tintRowHover(const QPixmap& row) const;

    QPixmap windowBox_;
    QPixmap rowBox_;
    QPixmap rowBoxHover_;
    QPixmap upArrow_;
    QPixmap upArrowHover_;
    QPixmap downArrow_;
    QPixmap downArrowHover_;
    QPixmap deleteIcon_;
    QPixmap deleteIconHover_;
    QPixmap coinIcon_;
    QPixmap clockIcon_;
    QPixmap coopBadge_;
};
