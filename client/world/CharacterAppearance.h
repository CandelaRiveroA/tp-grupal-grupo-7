#pragma once

#include <string>
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
    // Ataque de un enemigo: su propia secuencia one-shot (la enciende la
    // fuente de estado; hoy el server falso con la letra de prueba).
    Attack,
    // Daño recibido por un enemigo: su propia secuencia one-shot (la
    // enciende la fuente de estado). Al ser de ACCIÓN, el resolver la
    // anima aunque la entidad se esté moviendo (interrumpe la caminata
    // y, según la prioridad del que la emite, el ataque).
    Damage,
    // Muerte de un enemigo: su propia secuencia one-shot, que se queda
    // en el ÚLTIMO frame (loops = false). Terminal: no vuelve a Idle.
    Death,
};

struct ActionData {
    std::vector<Pose> frames;
    bool loops = false;
    int frameTicks = 0;
    // Hoja PROPIA de la acción (opcional): si está, la acción se dibuja
    // con esta hoja y su celda/escala, en vez de las de la criatura.
    // Vacío/0 = usar las de la criatura (el caso del jugador). Sirve para
    // enemigos con una hoja por acción (walk, attack, ...).
    std::string sheet;
    int cellWidth = 0;
    int cellHeight = 0;
    int frameCols = 0;
    int frameRows = 0;
};

}  // namespace world
