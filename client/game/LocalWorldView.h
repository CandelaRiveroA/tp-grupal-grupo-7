#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "common/model/draft/CharacterAppearance.h"
#include "game/GameConstants.h"
#include "game/SoundCue.h"
#include "world/FurnitureFacing.h"
#include "world/CreatureCatalog.h"
#include "world/CropCatalog.h"
#include "world/MarketOffer.h"
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

    // Personalización del personaje (índices de capa, tal cual los manda el
    // server). El render los acota contra characters.toml y cae al 0 si
    // están fuera de rango. Va al final por los aggregate-init posicionales.
    CharacterAppearance appearance{};
    // Selección pública del hotbar, distinta del contenido de una recogida.
    std::optional<std::string> carriedContent = std::nullopt;

    // Nombre que se muestra bajo los pies (el nombre de PERSONAJE elegido
    // al entrar, no el usuario del login). Lo llena la fuente de estado;
    // vacío = no se dibuja. Va al final por los aggregate-init posicionales.
    std::string displayName{};
};

// Recurso picable. La fuente decide contentId, celda y estado; al
// agotarse sale del vector resources.
//
// contentId: clave estable del contenido, vocabulario compartido con el
//            servidor (ej. "silver", "rock") y con resources.toml. El
//            cliente resuelve contentId -> asset (world::ResourceCatalog);
//            un id sin sección en el TOML no se dibuja.
struct ViewResource {
    std::string contentId;
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
// tradable:  si se puede regalar. Lo manda el servidor (ServerInventorySlot)
//            y es la fuente única de esa regla; true por defecto para que
//            las fuentes locales (mock, recompensas) sigan compiland sin
//            declararlo.
struct ViewInventorySlot {
    int slotIndex;
    std::string contentId;
    int count;
    bool tradable = true;
    // Calidad del contenido cuando es un producto cosechado (la manda la
    // fuente en ServerInventorySlot.quality, 1..5 BASE..PERFECT; default
    // Base mientras el server no la mande). Va al final del struct para
    // no romper los aggregate-init posicionales.
    world::CropQuality quality = world::CropQuality::Base;
    // Huella de piso si el contenido es un MUEBLE colocable (celdas
    // que ocupa). La manda la fuente de estado, como tradable: qué
    // ítems del jugador son muebles no es dato fijo del cliente. Sin
    // valor = no se coloca.
    std::optional<world::GridSpan> footprint = std::nullopt;
};

// Evento puntual del tick: la fuente informa que el jugador OBTUVO un
// contenido (no lo deduce el cliente). Se limpia cada tick. Usa el
// vocabulario genérico contentId (el mismo del inventario y los ítems
// del piso): el panel de obtenidos muestra cualquier cosa.
struct ViewObtained {
    std::string contentId;
};

// Pedido de regalo entrante (ServerGiftRequest): el emisor ofreció su ítem
// equipado y el server lo reenvía al receptor. Evento de UN tick (igual que
// obtained): el que lo dibuja abre el popup del receptor y responde con
// protocol::GiftResponseMessage (senderId = emisor a contestar, senderName
// = nombre real para el token {0} del copy).
struct ViewGiftRequest {
    std::uint8_t senderId;
    std::string senderName;
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

struct ViewChatLine {
    // Si el nombre está vacío, el que dibuja usa el id.
    std::uint8_t playerId;
    std::string characterName;
    std::string text;
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
// manda la fuente (el servidor: qué está plantado, en qué etapa, qué
// pide, con qué calidad se cosecha y si se puede levantar). El cliente
// solo lo dibuja y lo levanta donde la fuente lo permite (resolviendo
// cropId -> cultivo en world::CropCatalog).
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
    // Calidad de la cosecha (la manda la fuente en ServerCrop.quality; 1..5
    // BASE..PERFECT). Solo hace falta en la etapa final.
    world::CropQuality quality = world::CropQuality::Base;
    // ¿Se puede levantar (como el huevo)? Lo decide la FUENTE DE
    // ESTADO (el servidor dirá si se puede; hoy la demo lo permite en
    // la etapa final). El cliente solo lo usa para mostrar la
    // interacción y mandar la intención de tomarlo.
    bool interactive = false;
};

// Estado del sueño del jugador local (ServerSleep). Persiste entre lotes:
// el server solo lo manda cuando cambia. sleeping > 0 = estoy en la cama.
// Presentación (fase pendiente, la resuelve PopupState::showSleepState):
// sleeping > 0 sin votar y vivo → showQuestion(); votado o muerto →
// showWaiting(sleeping, total); sleeping == 0 → hide().
struct ViewSleep {
    int sleeping = 0;
    int total = 0;
};

// Dónde está parado un NPC del mercado. Es estado PERSISTENTE del mundo: lo
// manda la fuente de estado (el servidor). `id` es el de npcs.toml. El
// cliente solo lo dibuja y lo hace clickeable ahí; lo visual (hoja, retrato,
// texto) sigue siendo del TOML.
// Posición física de los pies, tal como llega del servidor.
struct ViewNpcPlacement {
    std::string id;
    int cellX;
    int cellY;
};

// Mueble que el jugador tiene colocado en el mundo (lo que consiguió y
// puso en la partida). Es estado PERSISTENTE: lo manda la fuente de
// estado (ServerFurniture) con el contentId y la celda; el cliente solo
// lo dibuja con el visual de ese contentId en inventory.toml. La celda
// es la base del mueble (abajo-izquierda del sprite).
//
// footprint:   celdas de PISO que ocupa (lo que bloquea), no la altura.
//              La manda la fuente; 1x1 por defecto.
// interactive: si se puede levantar (como el huevo). Lo decide la
//              fuente de estado (cerrado, sin impedimentos); el
//              cliente solo lo lee para el cursor y el Take.
struct ViewFurniture {
    std::string contentId;
    int cellX;
    int cellY;
    world::GridSpan footprint{1, 1};
    bool interactive = false;
    // Hacia dónde mira (la vista que lo dibuja). Lo manda la fuente; la
    // huella de arriba es la de frente: de costado se gira.
    world::FurnitureFacing facing = world::FurnitureFacing::Front;
    bool open = false;
};

// Posición global de un jugador para el mapa del mundo (tecla M).
// La fuente de estado (servidor o mock) la actualiza en cada tick.
struct ViewMapPlayer {
    int id = 0;
    std::string mapId;
    int cellX = 0;
    int cellY = 0;
    CharacterAppearance appearance{};
    bool isLocal = false;
};

// Aviso puntual para el jugador (evento, no estado): "qué pasó" por id
// ("inventory_full"). El texto y el aspecto viven en ui/notice.toml.
struct ViewNotice {
    std::string id;
};

struct LocalWorldView {
    std::vector<ViewEntity> entities;
    std::vector<ViewResource> resources;
    std::vector<ViewObtained> obtained;
    // Avisos del tick (p. ej. inventario lleno). Los llena la fuente de
    // estado a partir de los eventos enviados por el servidor.
    std::vector<ViewNotice> notices;
    // Pedido de regalo recibido en el tick (nullopt si no llegó ninguno).
    std::optional<ViewGiftRequest> giftRequest;
    // El receptor contestó MI pedido de regalo (ServerGiftAnswered):
    // evento, como dayEnded, vale solo en el tick en que llega.
    bool giftAnswered = false;
    std::vector<ViewDroppedItem> droppedItems;
    std::vector<ViewInventorySlot> inventory;
    world::GridSpan grid{0, 0};
    std::vector<ViewResourceHit> resourceHits;
    // El terreno con consecuencias aplicadas (persistente; ver
    // ViewGroundEffect). Lo manda el servidor (ServerGroundEffect).
    std::vector<ViewGroundEffect> groundEffects;
    // Los cultivos plantados (persistente; ver ViewCrop). Los manda el
    // servidor (ServerCrop).
    std::vector<ViewCrop> plantedCrops;
    // Posición de cada NPC (persistente; ver ViewNpcPlacement). La manda el
    // servidor. Vacío = cada NPC usa la celda `at` por defecto de npcs.toml;
    // un NPC que la fuente no menciona también usa la suya.
    std::vector<ViewNpcPlacement> npcPlacements;
    // Muebles colocados (persistente; ver ViewFurniture). Los manda el
    // servidor (ServerFurniture), por ejemplo al cargar una partida
    // guardada. Vacío = el cuarto solo tiene lo inicial de la escena.
    std::vector<ViewFurniture> placedFurniture;
    // Eventos de regalo aceptado: IDs de receptores; se vacían en cada update.
    std::vector<uint8_t> giftAcceptedPlayers;
    // Mapa en el que está el jugador (id del mapa del editor: "farm",
    // "town", "market"...). Lo manda la fuente de estado cuando hay
    // servidor. Vacío = el cliente elige la escena (teclas de prueba,
    // edificios tocados); con valor, la escena sigue a la fuente.
    std::string mapId;
    // Posición global de cada jugador conectado para el mapa del mundo (tecla M).
    // La llena la fuente de estado (servidor o mock) en cada tick.
    std::vector<ViewMapPlayer> worldMapPlayers;
    // Cues que emitieron entidades este tick (p. ej. la gallina cacarea).
    // Se limpian cada tick; la presentación los reproduce y los apaga.
    std::vector<ViewSoundEvent> soundEvents;
    ViewSleep sleep;
    // Día de juego (1 a 28) y hora: minutos desde las 6:00 (0 a 1200).
    // La hora cambia de a 10 min.
    int dayNumber = 1;
    int clockMinutes = 0;
    // Plata del jugador. La manda el servidor al comenzar y cada vez que
    // cambia; mientras no la mande vale la inicial local (kStartingGold).
    int gold = kStartingGold;
    // Se terminó el día (ServerDayEnd) y amaneció (ServerDay). Eventos: valen
    // solo en el tick en que llegan. Presentación: negro + popup de OK, y al
    // amanecer se levanta el negro con el gallo.
    bool dayEnded = false;
    bool dayChanged = false;
    // Eventos del Centro Comunitario; valen solo el tick en que llegan.
    uint8_t setCompleted = 0;
    bool victory = false;
    // Catálogo de un comerciante que el servidor pide mostrar (la respuesta a
    // /consultar_catalogo). EVENTO: vale solo en el tick en que llega; quien lo
    // llene lo limpia en cada update, como dayEnded. `merchantId` es el id de
    // npcs.toml; sin entradas = SDL muestra la oferta por defecto de ese
    // comerciante (market.toml); con entradas, esas (las que SDL conoce) con el
    // monto que trae cada una. SDL la ignora si ya hay un popup abierto.
    std::optional<world::MarketOffer> marketOffer;

    // Contenido del contenedor abierto (cofre): los slots ocupados, con el
    // mismo vocabulario que el inventario del jugador. Lo llena la fuente
    // de estado (el servidor); vacío = los cuadros se ven vacíos.
    std::vector<ViewInventorySlot> containerSlots;
    std::optional<world::GridCell> openContainer;

    // Vida y energía del jugador local (0..max).
    int playerHealth = 0;
    int playerMaxHealth = 0;
    int playerEnergy = 0;      // x100
    int playerMaxEnergy = 0;   // x100
    bool playerFainted = false;  // desmayado por falta de energía

    // Historial del chat
    std::vector<ViewChatLine> chatLog;

    // Sube con cada mensaje: el tope del historial impide usar su tamaño
    // para detectar uno nuevo.
    int chatVersion = 0;

    // Sube solo con los mensajes que llegan de OTRO jugador: no cuenta lo
    // que mandó el local (el servidor se lo devuelve) ni los avisos de
    // sistema. Lo lee ChatCue para avisar con un sonido.
    int incomingChatVersion = 0;

    // Suma un contenido al inventario (la regla genérica: apila por
    // contentId; si no estaba, ocupa el primer slot libre) y devuelve
    // el slot en el que quedó. El dueño de la vista (fuente de estado)
    // lo usa al entregar cosas.
    int addInventoryContent(const std::string& contentId);

    // Suma una línea al historial, descartando la más vieja si se pasa
    // del tope.
    void addChatLine(const ViewChatLine& line);

    // Coloca un contenido en un slot concreto con su conteo: para que el
    // que ya tenía slot lo mantenga merge tras merge (el inventario no
    // se reordena).
    void placeInventoryContent(int slotIndex, const std::string& contentId, int count);

    // ¿Está libre el slot en esta vista?
    bool inventorySlotFree(int slotIndex) const;
};

}  // namespace game
