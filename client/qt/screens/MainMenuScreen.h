#pragma once

#include <QString>

#include "qt/screens/Screen.h"

class QPushButton;

// Primera pantalla del lobby: logo y los tres accesos principales.
// Todavía sin estilo ni assets; el aspecto default de Qt es a propósito.
class MainMenuScreen : public Screen {
	Q_OBJECT

public:
	explicit MainMenuScreen(QWidget* parent = nullptr);

private:
	QPushButton* makeButton(const QString& text, ScreenId target);
};
