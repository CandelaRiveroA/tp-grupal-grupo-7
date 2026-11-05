#include "game/EnterSelection.h"

#include "game/GameConstants.h"
#include "world/ElementLayout.h"

namespace game {

EnterSelection::EnterSelection(const world::SceneDescription& scene,
			       const int cellPixelSize) {
	reindex(scene, cellPixelSize);
}

void EnterSelection::reindex(const world::SceneDescription& scene,
			     const int cellPixelSize) {
	entrables_.clear();
	const world::ScenePixelMapper mapper(cellPixelSize);
	for (std::size_t i = 0; i < scene.elements.size(); ++i) {
		const world::ElementPlacement& element = scene.elements[i];
		if (!element.enter.has_value()) {
			continue;
		}
		// La misma geometría que usa el renderer para el elemento:
		// lo que se toca es lo que se dibuja.
		const world::ElementLayout layout(
			scene.tilesets.at(element.tileset), element,
			mapper.toPixels(element.at, element.span));
		entrables_.push_back(Entrable{i, layout.bounds(), element.at,
					      element.span, *element.enter});
	}
}

EnterClick EnterSelection::clickAt(const world::PixelPoint& worldPoint,
				   const LocalWorldView& view) const {
	EnterClick click;
	const std::optional<std::size_t> found = entrableAt(worldPoint);
	if (!found.has_value()) {
		return click;
	}
	click.hit = true;
	const Entrable& entry = entrables_[*found];
	if (enteredIndex(view) == found) {
		// Ya está adentro: tocarla de nuevo no lo saca (la
		// pregunta la muestra quien llama).
		click.ask = true;
		return click;
	}
	if (near(entry, view)) {
		// En la zona de la cama (según la vista): se puede meter.
		click.ask = true;
		click.walkTo = entry.enter.lie;
	}
	// Lejos: el click no muestra nada ni lo mueve (la posición la
	// manda la fuente de estado; el popup no aparece desde cualquier
	// lado).
	return click;
}

bool EnterSelection::hits(const world::PixelPoint& worldPoint,
			  const LocalWorldView& view) const {
	const std::optional<std::size_t> found = entrableAt(worldPoint);
	if (!found.has_value()) {
		return false;
	}
	return enteredIndex(view) == found || near(entrables_[*found], view);
}

std::optional<EnteredFront> EnterSelection::front(
	const LocalWorldView& view) const {
	const std::optional<std::size_t> entered = enteredIndex(view);
	if (!entered.has_value()) {
		return std::nullopt;
	}
	const Entrable& entry = entrables_[*entered];
	return EnteredFront{entry.elementIndex, entry.enter.firstFrontRow,
			    entry.enter.lastFrontRow};
}

LocalWorldView EnterSelection::displayView(
	const LocalWorldView& view) const {
	const std::optional<std::size_t> entered = enteredIndex(view);
	if (!entered.has_value()) {
		return view;
	}
	// El jugador local "adentro": la pose del elemento (facing y
	// quieto). La posición es la REAL (está en la celda de la cama):
	// no hay nada que pisar.
	LocalWorldView placed = view;
	if (placed.entities.empty()) {
		return placed;
	}
	const world::ElementEnter& enter = entrables_[*entered].enter;
	ViewEntity& player = placed.entities[kLocalPlayerIndex];
	player.facing = enter.facing;
	player.moving = false;
	player.action = world::ActionKind::Idle;
	return placed;
}

bool EnterSelection::contains(const world::PixelRect& box,
			      const world::PixelPoint& point) const {
	return point.x >= box.x && point.x < box.x + box.width &&
	       point.y >= box.y && point.y < box.y + box.height;
}

std::optional<std::size_t> EnterSelection::entrableAt(
	const world::PixelPoint& worldPoint) const {
	// Orden inverso al de dibujo: si dos se superponen, gana el de
	// más arriba (el mismo criterio que ObjectPicker).
	for (std::size_t i = entrables_.size(); i > 0; --i) {
		const Entrable& entry = entrables_[i - 1];
		if (contains(entry.bounds, worldPoint)) {
			return i - 1;
		}
	}
	return std::nullopt;
}

std::optional<std::size_t> EnterSelection::enteredIndex(
	const LocalWorldView& view) const {
	if (view.entities.empty()) {
		return std::nullopt;
	}
	const ViewEntity& local = view.entities[kLocalPlayerIndex];
	for (std::size_t i = 0; i < entrables_.size(); ++i) {
		const world::GridCell& lie = entrables_[i].enter.lie;
		if (lie.column == local.cellX && lie.row == local.cellY) {
			return i;
		}
	}
	return std::nullopt;
}

bool EnterSelection::near(const Entrable& entry,
			  const LocalWorldView& view) const {
	if (view.entities.empty()) {
		return false;
	}
	const ViewEntity& local = view.entities[kLocalPlayerIndex];
	// Sobre el elemento o en el anillo de una celda alrededor.
	const int left = entry.at.column - 1;
	const int top = entry.at.row - 1;
	const int right = entry.at.column + entry.span.columns;
	const int bottom = entry.at.row + entry.span.rows;
	return local.cellX >= left && local.cellX <= right &&
	       local.cellY >= top && local.cellY <= bottom;
}

}  // namespace game
