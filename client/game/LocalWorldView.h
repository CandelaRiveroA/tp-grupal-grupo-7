#pragma once

#include <optional>
#include <string>
#include <vector>

#include "game/SoundCue.h"
#include "world/CreatureCatalog.h"
#include "world/CropCatalog.h"
#include "world/Grid.h"
#include "world/GroundCatalog.h"
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
    // Herramienta activa durante la ventana del golpe (UseTool, Water,
    // ...; la decide la fuente). nullopt si no usa herramienta.
    std::optional<world::ItemKind> activeTool;
    // Identificador estable de la entidad entre frames (lo asigna la
    // fuente). Permite seguirla aunque se mueva y referenciarla desde
    // eventos (p. ej. los cues de sonido).
    int id = 0;

    // Está en la cama (durmiendo), según el server (ServerSleepingPlayers).
    // No se deduce de la posición: parado en la cama no es durmiendo.
    bool sleeping = false;

    // ¿Es "apretable"/seleccionable? Lo decide la FUENTE DE ESTADO (el
    // servidor la mandará; hoy el mock lo simula). El cliente solo lo
    // usa para mostrar la interacción, no para decidir qué pasa.
    bool interactive = false;

    // Diálogo que la entidad muestra sobre sí misma (p. ej. la vaca
    // lista para ordeñar). Lo decide la FUENTE DE ESTADO; el cliente
    // solo lo anima y lo dibuja.
    std::optional<world::Indication> indication = std::nullopt;

    // Contenido sostenido en las manos durante la ventana de recogida
    // (el huevo, la cosecha; la decide la fuente): su contentId
    // genérico, dibujado subiendo con el overlay común ([held.*]).
    // nullopt si no se sostiene contenido. No confundir con
    // ActionResolver::heldContentId(), que es el carry local de la
    // selección del HUD (otra pose, otra fuente). Va al final del
    // struct para no romper los aggregate-init posicionales.
    std::optional<std::string> activeContent = std::nullopt;
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
    // ¿Es "apretable"/seleccionable? Lo decide la FUENTE DE ESTADO: el
    // servidor lo manda en ServerDropped.interactive y el mock lo simula,
    // igual que en las entidades.
    bool interactive = false;
};

// Impactos confirmados durante este tick; independientes del aspecto del recurso.
struct ViewResourceHit {
    int cellX;
    int cellY;
    bool destroyed;
};

// Consecuencia aplicada al terreno de una celda (tierra tratada, mañana
// regada): es estado PERSISTENTE del mundo, lo manda la fuente (el
// servidor decidirá qué piso tiene consecuencia; hoy el "server
// falso"). El cliente solo lo dibuja. El orden del vector es el de
// aplicación = el de pintado: una consecuencia se dibuja encima de la
// anterior sobre la misma celda.
struct ViewGroundEffect {
    int cellX;
    int cellY;
    world::GroundConsequence consequence;
};

// Cultivo plantado en una celda: es estado PERSISTENTE del mundo, lo
// manda la fuente (el servidor decidirá qué está plantado, en qué
// etapa, qué pide, con qué calidad se cosecha y si se puede levantar;
// hoy el "server falso"). El cliente solo lo dibuja y lo levanta donde
// la fuente lo permite (resolviendo cropId -> cultivo en
// world::CropCatalog).
struct ViewCrop {
    int cellX;
    int cellY;
    // Identidad del cultivo: el contentId de su PRODUCTO cosechado (lo
    // que se levanta es lo que entra al inventario).
    std::string cropId;
    // Etapa del ciclo de vida plantado: 0 = la semilla; 1..N =
    // crecimiento (world::CropDefinition::spriteForStage); la etapa
    // final (la cosecha) se dibuja con la calidad de abajo. La etapa la
    // decide la fuente; hoy arranca en 0 (se planta la semilla).
    int stage;
    // Diálogo que el cultivo muestra sobre sí mismo (p. ej. pide
    // agua). Lo decide la FUENTE DE ESTADO; el cliente solo lo anima y
    // lo dibuja (el mismo mecanismo que las entidades).
    std::optional<world::Indication> indication = std::nullopt;
    // Calidad de la cosecha (hoy la fuente manda la base; mañana el
    // servidor decidirá cuándo es increíble). Solo hace falta en la
    // etapa final.
    world::CropQuality quality = world::CropQuality::Base;
    // ¿Se puede levantar (como el huevo)? Lo decide la FUENTE DE
    // ESTADO (el servidor dirá si se puede; hoy la demo lo permite en
    // la etapa final). El cliente solo lo usa para mostrar la
    // interacción y mandar la intención de tomarlo.
    bool interactive = false;
};

// Estado del sueño del jugador local (ServerSleep). Persiste entre lotes:
// el server solo lo manda cuando cambia. sleeping > 0 = estoy en la cama.
// Presentación (fase pendiente): sleeping > 0 y sin votar → showQuestion();
// sleeping > 0 y votado → showWaiting(sleeping, total); sleeping == 0 → hide().
struct ViewSleep {
    int sleeping = 0;
    int total = 0;
};

struct LocalWorldView {
    std::vector<ViewEntity> entities;
    std::vector<ViewResource> resources;
    std::vector<ViewObtained> obtained;
    std::vector<ViewDroppedItem> droppedItems;
    std::vector<ViewInventorySlot> inventory;
    world::GridSpan grid{0, 0};
    std::vector<ViewResourceHit> resourceHits;
    // El terreno con consecuencias aplicadas (persistente; ver
    // ViewGroundEffect). Hoy lo llena el "server falso"; mañana el
    // servidor.
    std::vector<ViewGroundEffect> groundEffects;
    // Los cultivos plantados (persistente; ver ViewCrop). Hoy los llena
    // el "server falso"; mañana el servidor.
    std::vector<ViewCrop> plantedCrops;
    // Cues que emitieron entidades este tick (p. ej. la gallina cacarea).
    // Se limpian cada tick; la presentación los reproduce y los apaga.
    std::vector<ViewSoundEvent> soundEvents;
    ViewSleep sleep;
    // Día de juego (1 a 28) y hora: minutos desde las 6:00 (0 a 1200).
    // La hora cambia de a 10 min.
    int dayNumber = 1;
    int clockMinutes = 0;
    // Se terminó el día (ServerDayEnd) y amaneció (ServerDay). Eventos: valen
    // solo en el tick en que llegan. Presentación: negro + popup de OK, y al
    // amanecer se levanta el negro con el gallo.
    bool dayEnded = false;
    bool dayChanged = false;

    // Suma un contenido al inventario (la regla genérica: apila por
    // contentId; si no estaba, ocupa el primer slot libre) y devuelve
    // el slot en el que quedó. El dueño de la vista (fuente de estado)
    // lo usa al entregar cosas.
    int addInventoryContent(const std::string& contentId);

    // Coloca un contenido en un slot concreto con su conteo: para que el
    // que ya tenía slot lo mantenga merge tras merge (el inventario no
    // se reordena).
    void placeInventoryContent(int slotIndex, const std::string& contentId, int count);

    // ¿Está libre el slot en esta vista?
    bool inventorySlotFree(int slotIndex) const;
};

}  // namespace game
