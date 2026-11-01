#include "MainMenuScreen.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QTimerEvent>
#include <QVBoxLayout>

#include "core/MusicControl.h"
#include "qt/theme/CloudRects.h"
#include "qt/theme/TitleBackdrop.h"
#include "qt/theme/TitleSprites.h"
#include "qt/widgets/TitleButton.h"

MainMenuScreen::MainMenuScreen(QWidget* parent, core::MusicControl* music):
        Screen(parent), music_(music) {
    setObjectName("mainMenu");

    // La pantalla toma el foco al mostrarse (y no un boton)
    setFocusPolicy(Qt::StrongFocus);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, TITLE_LOGO_TOP_Y, 0, TITLE_BUTTON_ROW_BOTTOM_MARGIN);
    layout->setSpacing(0);

    auto* logo = new QLabel(this);
    logo->setObjectName("gameLogo");
    logo->setPixmap(sprites_.logo());
    logo->setFixedSize(ATLAS_LOGO_W * 2, ATLAS_LOGO_H * 2);
    logo->setAlignment(Qt::AlignCenter);
    layout->addWidget(logo, 0, Qt::AlignHCenter);

    layout->addStretch(1);

    // Botones sprite
    auto* buttons = new QWidget(this);
    buttons->setObjectName("menuButtons");
    auto* row = new QHBoxLayout(buttons);
    row->setContentsMargins(0, 0, 0, 0);
    row->setSpacing(TITLE_BUTTON_SPACING);
    row->addWidget(makeNavButton(0, "Nueva partida", "newButton", ScreenId::WorldSelect));
    row->addWidget(makeNavButton(1, "Reanudar partida", "loadButton", ScreenId::SaveList));
    row->addWidget(makeNavButton(2, "Unirse a partida", "coopButton", ScreenId::JoinCode));

    auto* exit = makeButton(3, "Salir", "exitButton");
    connect(exit, &QAbstractButton::clicked, this, &MainMenuScreen::exitRequested);
    row->addWidget(exit);
    layout->addWidget(buttons, 0, Qt::AlignHCenter);
}

TitleButton* MainMenuScreen::makeButton(int spriteIndex, const QString& accessibleText,
                                        const QString& name) {
    auto* button = new TitleButton(sprites_.button(spriteIndex), sprites_.buttonHover(spriteIndex),
                                   accessibleText, this);
    button->setObjectName(name);
    return button;
}

TitleButton* MainMenuScreen::makeNavButton(int spriteIndex, const QString& accessibleText,
                                           const QString& name, ScreenId target) {
    auto* button = makeButton(spriteIndex, accessibleText, name);
    connect(button, &QAbstractButton::clicked, this,
            [this, target]() { emit requestNavigate(target); });
    return button;
}

void MainMenuScreen::paintEvent(QPaintEvent* event) {
    QPainter painter(this);
    backdrop_.paint(painter);
    QWidget::paintEvent(event);
}

void MainMenuScreen::onEnter() {
    // Al volver devuelve el foco a la pantalla para no dejar ningun boton marcado.
    setFocus(Qt::OtherFocusReason);
    if (!cloudTimer_.isActive()) {
        cloudTimer_.start(CLOUD_ANIMATION_TICK_MS, this);
    }
    if (music_ != nullptr) {
        music_->play(core::MusicTrack::MenuTheme);
    }
}

void MainMenuScreen::onExit() {
    if (cloudTimer_.isActive()) {
        cloudTimer_.stop();
    }
    if (music_ != nullptr) {
        music_->stop();
    }
}

bool MainMenuScreen::isAnimating() const { return cloudTimer_.isActive(); }

void MainMenuScreen::timerEvent(QTimerEvent* event) {
    if (event->timerId() == cloudTimer_.timerId()) {
        backdrop_.step(float(CLOUD_ANIMATION_TICK_MS) / 1000.0f);
        update();
        return;
    }
    QWidget::timerEvent(event);
}
