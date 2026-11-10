#pragma once

#include <QImage>

// Carga una sola vez las hojas de personaje del .qrc.
class CharacterSheets {
public:
    CharacterSheets();

    const QImage& base() const;
    const QImage& skins() const;
    const QImage& hairstyles() const;
    const QImage& shirts() const;
    const QImage& pants() const;
    const QImage& accessories() const;

private:
    QImage load(const char* path) const;

    QImage base_;
    QImage skins_;
    QImage hairstyles_;
    QImage shirts_;
    QImage pants_;
    QImage accessories_;
};
