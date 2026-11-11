#include "world/SceneDescription.h"

namespace world {

bool ElementPlacement::visibleAt(const TimeOfDay time) const {
	// Sin `when` el elemento es de siempre; con él, solo se dibuja
	// en su momento. El render consulta esto por elemento tras el
	// índice espacial: qué hay y qué pasa no se toca.
	return !when.has_value() || *when == time;
}

}
