#pragma once

#include <QPixmap>

// Ensambla los sprites de la pantalla de unirse desde los recortes
class JoinCodeSprites {
public:
    JoinCodeSprites();

    const QPixmap& scrollBanner() const;
    const QPixmap& inputBox() const;
    const QPixmap& okButton() const;
    const QPixmap& okButtonHover() const;

private:
    QPixmap assembleScroll(const QPixmap& scroll) const;
    QPixmap brightenPercent(const QPixmap& sprite) const;
    QPixmap nullPixmap_;
    QPixmap scrollBanner_;
    QPixmap inputBox_;
    QPixmap okButton_;
    QPixmap okButtonHover_;
};
