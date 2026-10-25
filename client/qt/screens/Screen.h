#pragma once

#include <QWidget>

// Identifica cada página del lobby. El shell las registra y navega por id;
// las pantallas no se conocen entre sí.
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

	// Se llaman al entrar y al salir de la página. Vacías en la base.
	virtual void onEnter();
	virtual void onExit();

signals:
	// La pantalla pide ir a otra; no la muestra ella misma.
	void requestNavigate(ScreenId id);
};
