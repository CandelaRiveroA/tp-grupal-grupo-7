#include "CharacterSheets.h"

#include <QtGlobal>

CharacterSheets::CharacterSheets():
        base_(load(":/character/base.png")),
        skins_(load(":/character/skins.png")),
        hairstyles_(load(":/character/hairstyles.png")),
        shirts_(load(":/character/shirts.png")),
        pants_(load(":/character/pants.png")),
        accessories_(load(":/character/accessories.png")) {}

QImage CharacterSheets::load(const char* path) const {
    QImage image(path);
    if (image.isNull()) {
        qWarning("CharacterSheets: %s no carga (alias mal escrito en el .qrc o archivo ausente).",
                 path);
        return image;
    }
    return image.convertToFormat(QImage::Format_ARGB32);
}

const QImage& CharacterSheets::base() const { return base_; }

const QImage& CharacterSheets::skins() const { return skins_; }

const QImage& CharacterSheets::hairstyles() const { return hairstyles_; }

const QImage& CharacterSheets::shirts() const { return shirts_; }

const QImage& CharacterSheets::pants() const { return pants_; }

const QImage& CharacterSheets::accessories() const { return accessories_; }
