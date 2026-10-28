#pragma once

#include <QString>

#include "qt/screens/Screen.h"

class QPushButton;

class MainMenuScreen : public Screen {
	Q_OBJECT

public:
	explicit MainMenuScreen(QWidget* parent = nullptr);

signals:
	// "Salir" no navega a ninguna pantalla: el shell decide cómo cerrar.
	void exitRequested();

private:
	static constexpr int kButtonWidth = 200;

	QPushButton* makeButton(const QString& text, const QString& name);
	QPushButton* makeNavButton(const QString& text, const QString& name,
				   ScreenId target);
};
