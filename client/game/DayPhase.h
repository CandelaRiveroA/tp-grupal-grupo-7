#pragma once

#include <map>

#include "world/SceneDescription.h"

namespace game {

// Dueño del momento del día del cliente (día, atardecer, anochecer,
// noche). Es el punto de ENTRADA de esa información: hoy lo setean los
// atajos de prueba (teclas T, C y N, ver PlaySession::tick); mañana lo
// llama la conexión en vez de la tecla, con lo que el server mande (ver
// PopupState::showWaiting para el mismo patrón).
//
// Escalabilidad: cuando el server mande la hora, se suma ACÁ un setter
// que la traduzca a una etapa (tabla de umbrales). El render no cambia:
// sigue pidiendo phase() y los elementos siguen filtrando con `when`.
//
// La luz no salta entre etapas: nightFactor() persigue, un paso por tick
// (advance()), el valor de la etapa actual.
class DayPhase {
public:
	// `transitionTicks`: ticks que tarda la luz en recorrer de 0.0 a 1.0.
	explicit DayPhase(int transitionTicks);

	// Punto de ENTRADA de la fuente de estado: el momento del día.
	void setPhase(world::TimeOfDay phase);

	// Se llama una vez por tick: acerca la luz al valor de la etapa.
	void advance();

	// Lo que lee el render para filtrar los elementos con `when`: el
	// arte (Day o Night) que corresponde a la etapa actual.
	world::TimeOfDay phase() const;

	// Qué tan de noche se ve ahora, de 0.0 (día) a 1.0 (noche plena): lo
	// que modula la intensidad de la luz nocturna. Es gradual.
	double nightFactor() const;

private:
	world::TimeOfDay phase_;
	// Intensidad mostrada ahora; se mueve hacia la de la etapa.
	double level_;
	// Cuánto se mueve `level_` por tick.
	double step_;
	// Intensidad de noche por etapa (tabla, no switch).
	std::map<world::TimeOfDay, double> nightFactors_;
	// Arte (Day o Night) que se dibuja en cada etapa.
	std::map<world::TimeOfDay, world::TimeOfDay> lookPhases_;
};

}  // namespace game
