#pragma once

#include <QString>

#include "qt/screens/Screen.h"

class QPushButton;

class MainMenuScreen : public Screen {
	Q_OBJECT

public:
	explicit MainMenuScreen(QWidget* parent = nullptr);

private:
	QPushButton* makeButton(const QString& text, ScreenId target);
};
