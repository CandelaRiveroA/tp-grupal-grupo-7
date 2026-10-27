#include "game/ResourceAudioCue.h"

namespace game {

bool ResourceAudioCue::CellKey::operator<(const CellKey& other) const {
	if (x != other.x) {
		return x < other.x;
	}
	return y < other.y;
}

ResourceAudioEvent ResourceAudioCue::update(
	const std::vector<ViewResource>& resources) {
	std::map<CellKey, world::ResourceState> current;
	for (const ViewResource& r : resources) {
		current.emplace(CellKey{r.cellX, r.cellY}, r.state);
	}

	ResourceAudioEvent event = ResourceAudioEvent::None;

	// La primera observación solo fija el estado base: no dispara nada
	// (evita un falso evento al arrancar).
	if (known_) {
		// Un recurso que estaba y ya no está => se rompió del todo.
		// Tiene prioridad sobre Hit.
		for (const auto& [key, state] : previous_) {
			(void)state;
			if (current.find(key) == current.end()) {
				event = ResourceAudioEvent::Broke;
			}
		}
		// Si ninguno desapareció, buscar uno que haya cambiado de estado.
		if (event == ResourceAudioEvent::None) {
			for (const auto& [key, state] : current) {
				const auto it = previous_.find(key);
				if (it != previous_.end() && it->second != state) {
					event = ResourceAudioEvent::Hit;
					break;
				}
			}
		}
	}

	previous_ = std::move(current);
	known_ = true;
	return event;
}

}
