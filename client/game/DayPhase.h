#pragma once

#include <map>
#include <optional>
#include <string>

#include "world/SceneDescription.h"

namespace game {

// Dueño del momento del día del cliente (día, atardecer, anochecer,
// noche) y del reloj de juego que lo decide.
//
// Entrada normal: el reloj del server (ServerClock -> LocalWorldView ->
// setClock). De la hora se deriva la etapa con una tabla de umbrales, y
// de día + minutos los valores crudos que muestran las etiquetas del HUD.
//
// La luz no salta entre etapas: nightFactor() persigue, un paso por tick
// (advance()), el valor de la etapa actual.
class DayPhase {
public:
	// `transitionTicks`: ticks que tarda la luz en recorrer de 0.0 a 1.0.
	explicit DayPhase(int transitionTicks);

	// Entrada del server: día de juego (1 = lunes) y minutos desde las
	// 6:00. Actualiza los valores del HUD y, si la hora cruzó a otra
	// etapa, la aplica.
	void setClock(int dayNumber, int minutesSinceStart);

	// Se llama una vez por tick: acerca la luz al valor de la etapa.
	void advance();

	// Lo que lee el render para filtrar los elementos con `when`: el
	// arte (Day o Night) que corresponde a la etapa actual.
	world::TimeOfDay phase() const;

	// Qué tan de noche se ve ahora, de 0.0 (día) a 1.0 (noche plena): lo
	// que modula la intensidad de la luz nocturna. Es gradual.
	double nightFactor() const;

	// Valores crudos del reloj para las etiquetas del HUD (clave = token
	// de ui/hud.toml): día de la semana (0 = lunes), día, hora (24 h, puede
	// pasar de 23 después de medianoche) y minuto.
	const std::map<std::string, int>& clockValues() const;

private:
	// Aplica una etapa (la dicta setClock).
	void setPhase(world::TimeOfDay phase);

	// Etapa que corresponde a una hora del reloj (tabla de umbrales).
	world::TimeOfDay stageForHour(int hour) const;

	world::TimeOfDay phase_;
	// Intensidad mostrada ahora; se mueve hacia la de la etapa.
	double level_;
	// Cuánto se mueve `level_` por tick.
	double step_;
	// Intensidad de noche por etapa (tabla, no switch).
	std::map<world::TimeOfDay, double> nightFactors_;
	// Arte (Day o Night) que se dibuja en cada etapa.
	std::map<world::TimeOfDay, world::TimeOfDay> lookPhases_;
	// Hora (24 h) desde la que rige cada etapa.
	std::map<int, world::TimeOfDay> stageFromHour_;
	// Última etapa que dictó el reloj: solo se aplica al cambiar.
	std::optional<world::TimeOfDay> clockStage_;
	std::map<std::string, int> clockValues_;
};

}  // namespace game
