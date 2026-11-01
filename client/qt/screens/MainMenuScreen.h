#pragma once

#include <QBasicTimer>
#include <QString>

#include "qt/screens/Screen.h"
#include "qt/theme/TitleBackdrop.h"
#include "qt/theme/TitleSprites.h"

namespace core {
class MusicControl;
}

class QTimerEvent;
class TitleButton;

class MainMenuScreen: public Screen {
    Q_OBJECT

public:
    explicit MainMenuScreen(QWidget* parent = nullptr, core::MusicControl* music = nullptr);

    // Enciende/apaga la deriva de nubes, la musica y el foco de la pantalla
    void onEnter() override;
    void onExit() override;

    bool isAnimating() const;

signals:
    // "Salir" no navega a ninguna pantalla: el shell decide cómo cerrar.
    void exitRequested();

private:
    TitleButton* makeButton(int spriteIndex, const QString& accessibleText, const QString& name);
    TitleButton* makeNavButton(int spriteIndex, const QString& accessibleText, const QString& name,
                               ScreenId target);

    void paintEvent(QPaintEvent* event) override;
    void timerEvent(QTimerEvent* event) override;

    core::MusicControl* music_;
    TitleSprites sprites_;
    TitleBackdrop backdrop_;
    QBasicTimer cloudTimer_;
};
