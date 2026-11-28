#include "game/EnterSelection.h"

#include "game/AreaProximity.h"
#include "game/GameConstants.h"
#include "world/ElementLayout.h"

namespace game {

EnterSelection::EnterSelection(const world::SceneDescription& scene, const int cellPixelSize) {
    reindex(scene, cellPixelSize);
}

void EnterSelection::reindex(const world::SceneDescription& scene, const int cellPixelSize) {
    sceneEntrables_.clear();
    const world::ScenePixelMapper mapper(cellPixelSize);
    for (std::size_t i = 0; i < scene.elements.size(); ++i) {
        const world::ElementPlacement& element = scene.elements[i];
        if (!element.enter.has_value()) {
            continue;
        }
        // La misma geometría que usa el renderer para el elemento:
        // lo que se toca es lo que se dibuja.
        const world::ElementLayout layout(scene.tilesets.at(element.tileset), element,
                                          mapper.toPixels(element.at, element.span));
        sceneEntrables_.push_back(Entrable{EnterSource::Element, i, layout.bounds(), element.at,
                                           element.span, *element.enter});
    }
    entrables_ = sceneEntrables_;
}

void EnterSelection::observeFurniture(const LocalWorldView& view,
                                      const world::FurnitureGeometry& furniture,
                                      const int cellPixelSize) {
    entrables_ = sceneEntrables_;
    const world::ScenePixelMapper mapper(cellPixelSize);
    for (std::size_t i = 0; i < view.placedFurniture.size(); ++i) {
        const ViewFurniture& piece = view.placedFurniture[i];
        const world::NpcDef* prop = furniture.prop(piece.contentId);
        if (prop == nullptr || !prop->furnitureEnter.has_value()) {
            continue;
        }
        const world::GridCell source{piece.cellX, piece.cellY};
        const std::optional<world::FurnitureShape> shape =
                furniture.shapeOf(piece.contentId, source, mapper);
        if (!shape.has_value()) {
            continue;
        }
        const world::PropEnter& enter = *prop->furnitureEnter;
        entrables_.push_back(Entrable{
                EnterSource::Furniture, i, shape->box, shape->topLeft, shape->cells,
                world::ElementEnter{enter.firstFrontRow, enter.lastFrontRow, source,
                                    enter.facing}});
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

bool EnterSelection::hits(const world::PixelPoint& worldPoint, const LocalWorldView& view) const {
    const std::optional<std::size_t> found = entrableAt(worldPoint);
    if (!found.has_value()) {
        return false;
    }
    return enteredIndex(view) == found || near(entrables_[*found], view);
}

std::optional<EnteredFront> EnterSelection::front(const LocalWorldView& view) const {
    // La capa tapa a quien DUERME (lo dice el server), no a quien
    // solo está parado en la celda: si hay varios, la misma capa los
    // cubre a todos.
    for (const ViewEntity& entity: view.entities) {
        if (!entity.sleeping) {
            continue;
        }
        const std::optional<std::size_t> entered = entrableFor(entity.cellX, entity.cellY);
        if (entered.has_value()) {
            const Entrable& entry = entrables_[*entered];
            return EnteredFront{entry.source, entry.index, entry.enter.firstFrontRow,
                                entry.enter.lastFrontRow};
        }
    }
    return std::nullopt;
}

LocalWorldView EnterSelection::displayView(const LocalWorldView& view) const {
    // La pose es para quien DUERME (lo dice el server): en
    // multijugador el remoto se ve tapado en todas las pantallas. Su
    // posición es la REAL (está en la celda de la cama).
    LocalWorldView placed = view;
    for (ViewEntity& entity: placed.entities) {
        if (!entity.sleeping) {
            continue;
        }
        const std::optional<std::size_t> entered = entrableFor(entity.cellX, entity.cellY);
        if (!entered.has_value()) {
            continue;
        }
        const world::ElementEnter& enter = entrables_[*entered].enter;
        entity.facing = enter.facing;
        entity.moving = false;
        entity.action = world::ActionKind::Idle;
    }
    return placed;
}

bool EnterSelection::contains(const world::PixelRect& box, const world::PixelPoint& point) const {
    return point.x >= box.x && point.x < box.x + box.width && point.y >= box.y &&
           point.y < box.y + box.height;
}

std::optional<std::size_t> EnterSelection::entrableAt(const world::PixelPoint& worldPoint) const {
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

std::optional<std::size_t> EnterSelection::entrableFor(const int cellX, const int cellY) const {
    for (std::size_t i = 0; i < entrables_.size(); ++i) {
        const world::GridCell& lie = entrables_[i].enter.lie;
        if (lie.column == cellX && lie.row == cellY) {
            return i;
        }
    }
    return std::nullopt;
}

std::optional<std::size_t> EnterSelection::enteredIndex(const LocalWorldView& view) const {
    if (view.entities.empty()) {
        return std::nullopt;
    }
    const ViewEntity& local = view.entities[kLocalPlayerIndex];
    return entrableFor(local.cellX, local.cellY);
}

bool EnterSelection::near(const Entrable& entry, const LocalWorldView& view) const {
    if (view.entities.empty()) {
        return false;
    }
    const ViewEntity& local = view.entities[kLocalPlayerIndex];
    // Sobre el elemento o en el anillo de una celda alrededor.
    return AreaProximity(kInteractRing).around(entry.at, entry.span, local.cellX, local.cellY);
}

}  // namespace game
