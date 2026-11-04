#pragma once

#include <map>

#include "game/LocalWorldView.h"

namespace game {

// Reloj de los diálogos: recuerda desde cuándo cada entidad muestra su
// indicación para poder animarla. Mismo estilo que FootstepCue y
// ResourceAudioCue: estado puro, sin SDL, alimentado por la vista.
class DialogAnimator {
public:
	// Observa la vista: una entidad que empieza a mostrar una
	// indicación arranca su animación en este tick; las que dejan de
	// mostrarla (o desaparecen) se olvidan.
	void observe(const LocalWorldView& view, int iteration);

	// Ticks de animación de la entidad (0 si no tiene indicación).
	int animationTickFor(const ViewEntity& entity, int iteration) const;

private:
	// id estable de la entidad -> tick en que apareció su diálogo.
	std::map<int, int> startByEntityId_;
};

}  // namespace game
