#pragma once

#include <map>

#include "game/LocalWorldView.h"
#include "world/Grid.h"

namespace game {

// Reloj de los diálogos: recuerda desde cuándo cada entidad o cultivo
// muestra su indicación para poder animarla. Mismo estilo que
// FootstepCue y ResourceAudioCue: estado puro, sin SDL, alimentado por
// la vista.
class DialogAnimator {
public:
	// Observa la vista: lo que empieza a mostrar una indicación
	// arranca su animación en este tick; lo que deja de mostrarla (o
	// desaparece) se olvida.
	void observe(const LocalWorldView& view, int iteration);

	// Ticks de animación de la entidad (0 si no tiene indicación).
	int animationTickFor(const ViewEntity& entity, int iteration) const;

	// Ticks de animación del cultivo (0 si no tiene indicación). Su
	// identidad es la celda plantada (un cultivo por celda).
	int animationTickFor(const ViewCrop& crop, int iteration) const;

private:
	// Lleva el reloj de un dueño de diálogo a la lista nueva del tick:
	// si ya mostraba, conserva su tick de aparición; si aparece ahora,
	// arranca la animación de cero. Plantilla por la clave (el id de
	// la entidad o la celda del cultivo).
	template <typename Key>
	void carryStart(std::map<Key, int>& starts,
			const std::map<Key, int>& previous, const Key& key,
			int iteration) {
		const auto it = previous.find(key);
		starts[key] = it != previous.end() ? it->second : iteration;
	}

	// Ticks desde la aparición del diálogo del dueño (0 si no muestra
	// o su reloj no está).
	template <typename Key>
	int tickFor(const std::map<Key, int>& starts, const Key& key,
		    bool shown, int iteration) const {
		if (!shown) {
			return 0;
		}
		const auto it = starts.find(key);
		if (it == starts.end()) {
			return 0;
		}
		return iteration - it->second;
	}

	// id estable de la entidad -> tick en que apareció su diálogo.
	std::map<int, int> startByEntityId_;
	// celda del cultivo -> tick en que apareció su diálogo.
	std::map<world::GridCell, int> startByCropCell_;
};

}
