#pragma once

#include <vector>

#include "world/SpriteVisual.h"
#include "world/ViewDirection.h"

namespace world {


enum class Layer {
    Body,
    Arms,
    Clothing,
    Accessory,
};

struct LayerFrame {
    Layer layer;
    SheetCell frame;
    int cols = 1;
    int rows = 1;
};

using Pose = std::vector<LayerFrame>;

enum class ActionKind {
    Idle,
    Walk,
    UseTool,
    // Regar con la regadera: su propia secuencia de poses (no es el
    // golpe de UseTool). La enciende la fuente de estado con la
    // herramienta activa (tabla herramienta->acción, contrato §10).
    Water,
    Milk,
    Pickup,
    // Sostener un contenido soltable del inventario (brazos arriba con
    // el ítem sobre la cabeza). Es una pose LOCAL de la selección del
    // HUD (no la enciende el servidor): la resuelve game::ActionResolver
    // mientras el jugador local tenga seleccionado un contenido con el
    // rasgo `droppable`.
    Carry,
    // Caminar sosteniendo: las piernas de Walk con los brazos de Carry.
    // Variante de Walk para el mismo caso local (ver
    // ActionResolver::carryVariants_).
    CarryWalk,
};

struct ActionData {
    std::vector<Pose> frames;
    bool loops = false;
    int frameTicks = 0;
};

}  // namespace world
