#pragma once

#include <QString>

#include "qt/screens/Screen.h"
#include "qt/theme/TitleSprites.h"

namespace core {
class MusicControl;
}

class TitleButton;
class UsernamePlate;

class MainMenuScreen: public Screen {
    Q_OBJECT

public:
    explicit MainMenuScreen(QWidget* parent = nullptr, core::MusicControl* music = nullptr);

    // Enciende la musica y toma el foco; las nubes las maneja la base
    void onEnter() override;
    void onExit() override;

    signals:
    // "Salir" no navega a ninguna pantalla: el shell decide cómo cerrar.
    void exitRequested();

    void newGameRequested();

private:
    TitleButton* makeButton(int spriteIndex, const QString& accessibleText, const QString& name);
    TitleButton* makeNavButton(int spriteIndex, const QString& accessibleText, const QString& name,
                               ScreenId target);

    core::MusicControl* music_;
    TitleSprites sprites_;
    UsernamePlate* plate_;
};
