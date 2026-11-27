#pragma once

#include <array>
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
    // Desmayo: secuencia one-shot que termina tirado en el piso y se
    // queda en la última pose. La acción la enciende la fuente de
    // estado; mientras dure manda sobre Walk y Carry (el personaje
    // puede seguir moviéndose, pero la pose no cambia).
    Faint,
    // Tajo de espada: secuencia one-shot propia del jugador con la
    // herramienta espada (tabla toolActions_ en ActionResolver). La
    // enciende la fuente con UseTool + activeTool = Sword.
    Sword,
    // Revivir: la animación de muerte de la criatura, AL REVÉS (los
    // frames de death invertidos, characters.toml `frames_from` +
    // `reverse`). La enciende la fuente; termina en la pose viva.
    Revive,
    Run,
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

// Cara de una capa de apariencia. Izquierda y derecha comparten `Side`:
// se dibuja la misma celda y el render la espeja según la dirección.
enum class AppearanceSide {
    Front,
    Side,
    Back,
    Count,
};

// Contra qué se posiciona el `offset` de una capa. `Body` (por defecto) es
// el cuerpo; `Head` sigue la coronilla (lo que va sobre la cabeza) y `Feet`
// sigue los pies (lo que va sobre las piernas). Así cada pieza de ropa
// acompaña a la parte del cuerpo que le corresponde cuando se mueve.
enum class AppearanceAnchor {
    Body,
    Head,
    Feet,
    Count,
};

// Una opción de una capa (un peinado, una camisa, un pantalón, un
// accesorio). `sides[side]` es la lista de cuadros de esa cara: las capas
// estáticas tienen uno; el pantalón tiene tres (idle/walk). Una lista
// vacía significa que esa cara no se dibuja (p. ej. accesorios de espalda).
struct AppearanceOption {
    std::array<std::vector<SheetRegion>,
               static_cast<std::size_t>(AppearanceSide::Count)>
        sides;
};

// Capa de apariencia: hoja, grilla de celdas, offset nativo contra el
// cuerpo y una opción por índice del server. Todo es dato: sumar un
// peinado/camisa/accesorio es agregar una opción al TOML.
struct AppearanceLayerData {
    std::string sheetImage;
    int cellWidth = 0;
    int cellHeight = 0;
    // Contra qué se posiciona el offset (cuerpo o coronilla).
    AppearanceAnchor anchor = AppearanceAnchor::Body;
    // Offset de dibujo por cara (Front/Side/Back). El `offset` del TOML
    // los inicializa a todos iguales; `offset_front/side/back` (opcionales)
    // ajustan una cara concreta — p. ej. el pantalón de costado se alinea
    // 1px distinto. El offset.x es relativo al cuerpo; el offset.y es
    // relativo al cuerpo (anchor=Body) o a la coronilla (anchor=Head).
    std::array<std::array<int, 2>,
               static_cast<std::size_t>(AppearanceSide::Count)>
        offset{};
    std::vector<AppearanceOption> options;
};

// Piel: paleta de filas (skinColors..png). La fila 0 son los colores
// testigo que ya trae el cuerpo; la fila N, el color elegido.
struct SkinAppearanceData {
    std::string paletteSheet;
    int cellWidth = 0;
    int cellHeight = 0;
    int count = 0;
};

// Apariencia completa del jugador (tabla opcional en characters.toml).
// `present` distingue "no tiene apariencia" (animales) de una tabla
// cargada pero vacía.
struct AppearanceData {
    bool present = false;
    SkinAppearanceData skin;
    AppearanceLayerData hair;
    AppearanceLayerData shirt;
    AppearanceLayerData pants;
    AppearanceLayerData accessory;
};

}  // namespace world
