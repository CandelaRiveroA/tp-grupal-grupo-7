#pragma once

#include <map>
#include <vector>

#include "game/LocalWorldView.h"
#include "world/ResourceCatalog.h"

namespace game {

// Evento de audio por el cambio VISUAL de los recursos entre frames
// (presentación). La OBTENCIÓN del mineral NO se deduce acá: la manda la
// fuente (LocalWorldView.obtained), no la infiere el cliente.
enum class ResourceAudioEvent {
	None,
	Hit,
	Broke,
};

// Observa los recursos entre frames y dice qué evento de audio disparar,
// análogo a FootstepCue. No reproduce sonido: solo decide.
class ResourceAudioCue {
public:
	ResourceAudioEvent update(const std::vector<ViewResource>& resources);

private:
	// Clave por celda (el servidor mandará un id propio; por ahora la
	// celda identifica al recurso).
	struct CellKey {
		int x;
		int y;
		bool operator<(const CellKey& other) const;
	};

	std::map<CellKey, world::ResourceState> previous_;
	bool known_ = false;
};

}
