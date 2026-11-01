#include "GameCursor.h"

#include <QPixmap>

#include "CursorRects.h"

GameCursor::GameCursor() {
    QPixmap arrow(":/ui/cursor_arrow.png");
    if (arrow.isNull()) {
        qWarning("GameCursor: :/ui/cursor_arrow.png no carga "
                 "(alias mal escrito en el .qrc o archivo ausente); "
                 "se usa la flecha por defecto de Qt.");
        cursor_ = QCursor(Qt::ArrowCursor);
        return;
    }

    const QPixmap scaled =
            arrow.scaled(CURSOR_ARROW_SIZE * CURSOR_SCALE, CURSOR_ARROW_SIZE * CURSOR_SCALE,
                         Qt::IgnoreAspectRatio, Qt::FastTransformation);
    cursor_ = QCursor(scaled, CURSOR_HOTSPOT_X * CURSOR_SCALE, CURSOR_HOTSPOT_Y * CURSOR_SCALE);
}

const QCursor& GameCursor::cursor() const { return cursor_; }
