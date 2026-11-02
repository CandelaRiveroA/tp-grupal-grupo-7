#pragma once

#include <QBasicTimer>
#include <QWidget>

#include "qt/theme/TitleBackdrop.h"

enum class ScreenId {
	MainMenu,
	WorldSelect,
	Character,
	SaveList,
	JoinCode,
};

// Estado de una pantalla que espera datos de un servicio asíncrono.
// No todas las pantallas usan todos los valores.
enum class ScreenState {
	Idle,
	Loading,
	Ready,
	Empty,
	Error,
};

// Base de las pantallas del lobby. El shell la muestra dentro de un
// QStackedWidget y llama a onEnter()/onExit() al cambiar de página.
// Todas comparten el cielo con la deriva de nubes.
class Screen : public QWidget {
	Q_OBJECT

public:
	explicit Screen(QWidget* parent = nullptr);

	virtual void onEnter();
	virtual void onExit();

signals:
	// La pantalla pide ir a otra; no la muestra ella misma.
	void requestNavigate(ScreenId id);

protected:
	// Pinta el cielo. Las subclases que pinten encima lo llaman primero.
	void paintEvent(QPaintEvent* event) override;
	void timerEvent(QTimerEvent* event) override;

private:
	TitleBackdrop backdrop_;
	QBasicTimer cloudTimer_;
};