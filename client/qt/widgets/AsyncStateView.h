#pragma once

#include <map>
#include <vector>

#include "qt/screens/Screen.h"

class QWidget;

// Muestra y oculta los widgets que registra cada pantalla según el estado.
// No es dueño de nada: no crea, ni borra, ni reparentea; solo cambia la
// visibilidad de lo registrado. Un widget puede responder a varios estados
// (el mismo status sirve para carga, vacío y error).
class AsyncStateView {
public:
	AsyncStateView();

	// Registra un widget visible en ese estado. Un estado puede tener
	// varios widgets y también puede quedarse sin ninguno.
	void bind(ScreenState state, QWidget* widget);
	void setState(ScreenState state) const;

private:
	// Clave por widget y no por estado: los mismos widgets atienden a
	// varios estados y el registro no se pisa a sí mismo.
	std::map<QWidget*, std::vector<ScreenState>> bindings_;
};
