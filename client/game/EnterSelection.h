#pragma once

#include <cstddef>
#include <optional>
#include <vector>

#include "game/LocalWorldView.h"
#include "world/SceneDescription.h"
#include "world/ScenePixelMapper.h"

namespace game {

// Qué pide el click sobre un elemento entrable (solo datos). Qué hacer
// con eso lo decide quien llama.
struct EnterClick {
    // El click fue sobre un elemento entrable (consumido: no cae al
    // mundo).
    bool hit = false;

    // Mostrar la pregunta del popup: solo si el jugador está en la
    // zona de la cama SEGÚN LA VISTA (la posición la manda la fuente
    // de estado). Es el gate para que el popup no aparezca desde
    // cualquier lado de la sala.
    bool ask = false;

    // Celda a la que caminar para "meterse". Vacío si ya está adentro
    // o si está lejos: la cama no manda al jugador caminando al otro
    // lado de la sala, se acerca él.
    std::optional<world::GridCell> walkTo;
};

// La capa frontal a dibujar de un elemento entrable: el elemento (índice
// en la escena) y el rango de filas de su pieza que va ENCIMA de las
// entidades. DTO plano.
struct EnteredFront {
    std::size_t elementIndex;
    int firstRow;
    int lastRow;
};

// Presentación local de "meterse" en un elemento entrable (el que
// declara `enter` en scenes.toml): las filas frontales tapan al jugador
// (en la cama: las frazadas) y su pose es la del elemento.
//
// El estado es DERIVADO: el jugador está "adentro" mientras su celda es
// la del elemento (`lie`); al caminar, sale solo. La posición no se pisa:
// "meterse" es caminar hasta ahí (PlayerController::walkTo), así después
// camina libre desde la cama, sin saltos. Es un efecto visual del
// cliente: la fuente de estado no participa.
class EnterSelection {
public:
    // Indexa los elementos entrables de la escena con las MISMAS
    // cajas que el renderer (ElementLayout), para que lo que se toca
    // coincida con lo que se ve.
    EnterSelection(const world::SceneDescription& scene, int cellPixelSize);

    // Re-indexa con la escena nueva (al cambiar de escena): los
    // índices de los entrables siempre son de la escena actual.
    void reindex(const world::SceneDescription& scene, int cellPixelSize);

    // Click en px de mundo: qué pide el click (ver EnterClick).
    EnterClick clickAt(const world::PixelPoint& worldPoint, const LocalWorldView& view) const;

    // ¿Hay un elemento entrable apretable bajo el punto (variante del
    // cursor)? Apretable = el click hace algo: está cerca o ya adentro.
    bool hits(const world::PixelPoint& worldPoint, const LocalWorldView& view) const;

    // Capa frontal a dibujar encima de las entidades: las filas que
    // tapan a quien DUERME (ViewEntity::sleeping, dato del server) en
    // una cama. Nadie durmiendo = nada.
    std::optional<EnteredFront> front(const LocalWorldView& view) const;

    // La vista a dibujar: con cada entidad que duerme en la pose del
    // elemento (facing y quieto). La posición NO se toca.
    LocalWorldView displayView(const LocalWorldView& view) const;

private:
    struct Entrable {
        std::size_t elementIndex;
        world::PixelRect bounds;
        world::GridCell at;
        world::GridSpan span;
        world::ElementEnter enter;
    };

    bool contains(const world::PixelRect& box, const world::PixelPoint& point) const;

    // Índice del entrable bajo el punto; si varios se superponen,
    // gana el de más arriba en el orden de dibujo (el último).
    std::optional<std::size_t> entrableAt(const world::PixelPoint& worldPoint) const;

    // Índice del elemento cuya celda lie es la dada (para cualquier
    // celda; sirve para el local y para los remotos).
    std::optional<std::size_t> entrableFor(int cellX, int cellY) const;

    // Índice del elemento donde está el jugador local (su celda es la
    // lie). Solo del local: los clicks son intenciones suyas.
    std::optional<std::size_t> enteredIndex(const LocalWorldView& view) const;

    // ¿El jugador está sobre el elemento o al lado (anillo de 1
    // celda)? La cercanía sale de los datos del elemento (at/span).
    bool near(const Entrable& entry, const LocalWorldView& view) const;

    std::vector<Entrable> entrables_;
};

}  // namespace game
