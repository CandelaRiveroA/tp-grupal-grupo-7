#pragma once

#include <QWidget>

enum class ScreenId {
	MainMenu,
	WorldSelect,
	Character,
	SaveList,
	JoinCode,
};

// Base de las pantallas del lobby. El shell la muestra dentro de un
// QStackedWidget y llama a onEnter()/onExit() al cambiar de página.
class Screen : public QWidget {
	Q_OBJECT

public:
	explicit Screen(QWidget* parent = nullptr);

	virtual void onEnter();
	virtual void onExit();

signals:
	// La pantalla pide ir a otra; no la muestra ella misma.
	void requestNavigate(ScreenId id);
};
