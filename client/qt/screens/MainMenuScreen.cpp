#include "MainMenuScreen.h"

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

MainMenuScreen::MainMenuScreen(QWidget* parent) : Screen(parent) {
	setObjectName("mainMenu");

	auto* layout = new QVBoxLayout(this);
	layout->setContentsMargins(96, 64, 96, 64);
	layout->addStretch();

	auto* logo = new QLabel("Stardew Valley", this);
	logo->setObjectName("gameLogo");
	layout->addWidget(logo);

	layout->addSpacing(48);

	layout->addWidget(makeButton("Nueva partida", ScreenId::WorldSelect), 0,
			  Qt::AlignLeft);
	layout->addWidget(makeButton("Retomar partida", ScreenId::SaveList), 0,
			  Qt::AlignLeft);
	layout->addWidget(makeButton("Unirse a partida", ScreenId::JoinCode), 0,
			  Qt::AlignLeft);

	layout->addStretch();
}

QPushButton* MainMenuScreen::makeButton(const QString& text, ScreenId target) {
	auto* button = new QPushButton(text, this);
	button->setFixedWidth(280);
	connect(button, &QPushButton::clicked, this,
		[this, target]() { emit requestNavigate(target); });
	return button;
}
