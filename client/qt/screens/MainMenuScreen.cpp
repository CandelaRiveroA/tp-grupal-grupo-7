#include "MainMenuScreen.h"

#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

MainMenuScreen::MainMenuScreen(QWidget* parent) : Screen(parent) {
	setObjectName("mainMenu");

	auto* layout = new QVBoxLayout(this);
	layout->setContentsMargins(96, 64, 96, 64);
	layout->addStretch();

	// El logo va en su propio panel, centrado, como el cartel del juego.
	auto* banner = new QFrame(this);
	banner->setObjectName("logoBanner");
	auto* bannerLayout = new QVBoxLayout(banner);
	auto* logo = new QLabel("Stardew Valley", banner);
	logo->setObjectName("gameLogo");
	logo->setAlignment(Qt::AlignCenter);
	bannerLayout->addWidget(logo);
	layout->addWidget(banner, 0, Qt::AlignHCenter);

	layout->addStretch();

	auto* buttons = new QWidget(this);
	buttons->setObjectName("menuButtons");
	auto* row = new QHBoxLayout(buttons);
	row->setContentsMargins(0, 0, 0, 0);
	row->setSpacing(32);
	row->addWidget(makeNavButton("Nueva partida", "newButton",
				     ScreenId::WorldSelect));
	row->addWidget(makeNavButton("Reanudar partida", "loadButton",
				     ScreenId::SaveList));
	row->addWidget(makeNavButton("Unirse a partida", "coopButton",
				     ScreenId::JoinCode));

	auto* exit = makeButton("Salir", "exitButton");
	connect(exit, &QPushButton::clicked, this,
		&MainMenuScreen::exitRequested);
	row->addWidget(exit);
	layout->addWidget(buttons, 0, Qt::AlignHCenter);

	layout->addStretch();
}

QPushButton* MainMenuScreen::makeButton(const QString& text,
					const QString& name) {
	auto* button = new QPushButton(text, this);
	button->setObjectName(name);
	button->setFixedWidth(kButtonWidth);
	return button;
}

QPushButton* MainMenuScreen::makeNavButton(const QString& text,
					   const QString& name,
					   ScreenId target) {
	auto* button = makeButton(text, name);
	connect(button, &QPushButton::clicked, this,
		[this, target]() { emit requestNavigate(target); });
	return button;
}
