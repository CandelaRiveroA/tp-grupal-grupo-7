#pragma once

#include <string>

namespace game {

// La transición de escena "apagado y vuelta a prender": fade a negro,
// en el negro cambian los datos y fade de vuelta. Presentación pura
// (sin SDL): el dibujo es un rect con el alfa que esto expone. Sirve
// para el próximo día, para salir de la habitación o para cualquier
// cambio de escenario, siempre en la misma ventana.
//
// Punto de ENTRADA de la fuente de estado: `changeTo(...)` — "cambio de
// escenario". Hoy lo llama el atajo de prueba (tecla B); mañana lo llama
// el GameClient de red cuando el server lo decida. No conoce input ni
// red: borrar el atajo no toca esta clase.
class SceneTransition {
public:
	// fadeTicks: los ticks de cada mitad del fade (los del arranque).
	explicit SceneTransition(int fadeTicks);

	// Pide el cambio a la escena destino. Se ignora si ya hay una
	// transición en curso (una a la vez).
	void changeTo(const std::string& scene);

	// Avanza el reloj de la transición (una vez por tick).
	void tick(int iteration);

	bool active() const;

	// El negro a dibujar sobre todo lo demás: 0 = nada, 255 = negro
	// total (el instante en que cambian los datos).
	int alpha() const;

	// ¿Llegó el instante del negro (el momento de cambiar los datos)?
	bool swapDue() const;

	// Da por hecho el cambio de datos (una sola vez por transición).
	void consumeSwap();

	// La escena destino del cambio en curso.
	const std::string& scene() const;

private:
	enum class Phase {
		Idle,
		FadingOut,
		FadingIn,
	};

	int fadeTicks_;
	Phase phase_ = Phase::Idle;
	std::string scene_;
	int startIteration_ = -1;
	int alpha_ = 0;
	bool swapPending_ = false;
};

}  // namespace game
