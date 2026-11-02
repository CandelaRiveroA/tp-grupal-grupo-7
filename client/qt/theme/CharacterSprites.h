#pragma once

#include <QPixmap>

// Ensambla una sola vez los sprites de la pantalla de personaje desde los
// recortes; la ventana central (9-slice) es windowBox().
class CharacterSprites {
public:
    CharacterSprites();

    const QPixmap& portraitBackground() const;
    const QPixmap& leftArrow() const;
    const QPixmap& leftArrowHover() const;
    const QPixmap& rightArrow() const;
    const QPixmap& rightArrowHover() const;
    const QPixmap& okButton() const;
    const QPixmap& okButtonHover() const;
    const QPixmap& inputBox() const;
    const QPixmap& windowBox() const;

private:
    QPixmap brightenPercent(const QPixmap& sprite) const;
    QPixmap nullPixmap_;
    QPixmap portraitBackground_;
    QPixmap leftArrow_;
    QPixmap leftArrowHover_;
    QPixmap rightArrow_;
    QPixmap rightArrowHover_;
    QPixmap okButton_;
    QPixmap okButtonHover_;
    QPixmap inputBox_;
    QPixmap windowBox_;
};
