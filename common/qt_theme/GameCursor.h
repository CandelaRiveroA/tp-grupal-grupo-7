#pragma once

#include <QCursor>

class GameCursor {
public:
    GameCursor();

    const QCursor& cursor() const;

private:
    QCursor cursor_;
};
