#pragma once

#include <optional>
#include <string>
#include <vector>

#include "game/SoundCue.h"
#include "world/CreatureCatalog.h"
#include "world/Grid.h"
#include "world/Indication.h"
#include "world/ItemKind.h"
#include "world/ResourceCatalog.h"
#include "world/ViewDirection.h"

namespace game {


struct ViewEntity {
    world::CreatureKind kind;
    int cellX;
    int cellY;
    world::Direction facing;
    bool moving;
    world::ActionKind action = world::ActionKind::Idle;
    // Herramienta activa durante UseTool (la decide la fuente). nullopt
    // si no usa herramienta.
    std::optional<world::ItemKind> activeTool;
    // Identificador estable de la entidad entre frames (lo asigna la
    // fuente). Permite seguirla aunque se mueva y referenciarla desde
    // eventos (p. ej. los cues de sonido).
    int id = 0;

    // ¿Es "apretable"/seleccionable? Lo decide la FUENTE DE ESTADO (el
    // servidor la mandará; hoy el mock lo simula). El cliente solo lo
    // usa para mostrar la interacción, no para decidir qué pasa.
    bool interactive = false;

    // Diálogo que la entidad muestra sobre sí misma (p. ej. la vaca
    // lista para ordeñar). Lo decide la FUENTE DE ESTADO; el cliente
    // solo lo anima y lo dibuja.
    std::optional<world::Indication> indication = std::nullopt;
};

// Recurso picable. La fuente decide tipo, celda y estado; al agotarse
// sale del vector resources.
struct ViewResource {
    world::ResourceKind kind;
    int cellX;
    int cellY;
    world::ResourceState state;
};

// Un slot del inventario. El inventario es GENÉRICO: guarda cualquier
// contenido (hoy herramientas y minerales, mañana huevos u otros) mediante
// un identificador abstracto. La fuente (servidor) decide qué contentId va
// en cada slot y en qué cantidad; el cliente resuelve contentId -> asset
// (world::ContentCatalog) y dibuja. No depende de ItemKind.
//
// contentId: clave estable del contenido, vocabulario compartido con el
//            servidor (ej. "pickaxe", "silver", futuro "egg").
// count:     cantidad acumulada en el slot. Convención de dibujo: 1 no
//            muestra número; >1 muestra el número.
struct ViewInventorySlot {
    int slotIndex;
    std::string contentId;
    int count;
};

// Evento puntual del tick: la fuente informa que el jugador OBTUVO un
// contenido (no lo deduce el cliente). Se limpia cada tick. Usa el
// vocabulario genérico contentId (el mismo del inventario y los ítems
// del piso): el panel de obtenidos muestra cualquier cosa.
struct ViewObtained {
    std::string contentId;
};

// Item caído al piso, recogible por el jugador. GENÉRICO: lo identifica un
// contentId (string, mismo vocabulario que el inventario y el servidor), no
// un tipo concreto. Hoy caen minerales y productos de granja (huevos,
// leche), sin cambiar esta estructura. La fuente decide contentId y celda
// (no se deduce); el cliente lo dibuja en el piso resolviendo contentId ->
// asset (world::ContentCatalog). Al recogerse (comando Take sobre su celda)
// sale del vector y se emite obtained.
struct ViewDroppedItem {
    std::string contentId;
    int cellX;
    int cellY;
    // ¿Es "apretable"/seleccionable? Lo decide la FUENTE DE ESTADO (el
    // servidor lo mandará; hoy el mock lo simula), igual que en las
    // entidades.
    bool interactive = false;
};

// Impactos confirmados durante este tick; independientes del aspecto del recurso.
struct ViewResourceHit {
    int cellX;
    int cellY;
    bool destroyed;
};

struct LocalWorldView {
    std::vector<ViewEntity> entities;
    std::vector<ViewResource> resources;
    std::vector<ViewObtained> obtained;
    std::vector<ViewDroppedItem> droppedItems;
    std::vector<ViewInventorySlot> inventory;
    world::GridSpan grid{0, 0};
    std::vector<ViewResourceHit> resourceHits;
    // Cues que emitieron entidades este tick (p. ej. la gallina cacarea).
    // Se limpian cada tick; la presentación los reproduce y los apaga.
    std::vector<ViewSoundEvent> soundEvents;

    // Suma un contenido al inventario (la regla genérica: apila por
    // contentId; si no estaba, ocupa el primer slot libre) y devuelve
    // el slot en el que quedó. El dueño de la vista (fuente de estado)
    // lo usa al entregar cosas.
    int addInventoryContent(const std::string& contentId);

    // Coloca un contenido en un slot concreto con su conteo: para que el
    // que ya tenía slot lo mantenga merge tras merge (el inventario no
    // se reordena).
    void placeInventoryContent(int slotIndex, const std::string& contentId,
                               int count);

    // ¿Está libre el slot en esta vista?
    bool inventorySlotFree(int slotIndex) const;
};

}  // namespace game
