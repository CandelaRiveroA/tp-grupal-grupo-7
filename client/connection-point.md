# Punto de conexión cliente ↔ servidor

Documento para coordinar con quien hace **protocolos**. Resume qué manda el
cliente, qué recibe, y **dónde exactamente** se conecta la red.

El cliente es **dummy** (ver `Semana 2.md`): no decide la lógica de juego.
Manda *intenciones* y dibuja el *estado* que el servidor devuelve. El cliente
SDL se conecta al servidor real con `game::NetworkGameClient`; `MockGameClient`
queda solo para tests y desarrollo sin servidor.

```
InputManager ──► MoveCommand ──► GameClient (interfaz = punto de conexión)
                                  │  sendCommand(ClientMessage)   [cliente → servidor]
                                  │  worldState()                  [servidor → cliente]
                                  ▼
      NetworkGameClient  (lo que corre en la app: socket + protocolo)
      MockGameClient     (solo tests / desarrollo sin servidor)
```

---

## 1. Qué MANDA el cliente — `protocol::ClientMessage`

Archivo: `common/common_command.h`. El cliente envía **intenciones** por
dirección; el servidor decide qué pasa (colisiones, herramienta, inventario).

```cpp
enum class ClientMessageType : uint8_t { Move = 1, Pick = 2, Take = 3, Sleep = 4, Wake = 5, Chat = 6, GiftResponse = 7, ToggleWalking = 8 };

struct MoveMessage { Direction direction; };  // caminar; fija el facing
struct PickMessage { Direction direction; uint8_t slot; };  // usar el casillero `slot` sobre la celda enfrentada
struct TakeMessage { Direction direction; uint8_t slot; };  // recoger en la celda propia; `slot` = casillero elegido (como en Pick)
struct SleepMessage { bool accept; };         // voto del aviso de la cama
struct WakeMessage {};                       // OK del popup del nuevo día
struct ChatMessage { std::string text; };    // texto libre: broadcast, @nombre privado o /comando
struct GiftResponseMessage { uint8_t sender_id; bool accept; };  // responde el pedido de regalo de `sender_id`
struct ToggleWalkingMessage {}; // alterna el modo de 1 o 2 celdas por movimiento

using ClientMessage = std::variant<MoveMessage, PickMessage, TakeMessage, SleepMessage, WakeMessage, ChatMessage, GiftResponseMessage, ToggleWalkingMessage>;
```

- `Direction` (global, `common/common_constants.h`): `UP=0, DOWN=1, LEFT=2, RIGHT=3`.
- El Centro Comunitario usa chat:
  `/donar [cultivos|minerales|animales] [item] [cantidad]`,
  `/donar dinero [cantidad]`, `/completar_set [nombre]` y
  `/completar_todo`. Los sets tienen IDs 1 (`cultivos`), 2 (`minerales`),
  3 (`animales`), 4 (`pelea`) y 5 (`dinero`). `ServerSetCompleted` lleva
  `set_id u8`; `ServerVictory` no lleva payload.
- Quedarse parado **no envía nada**.
- Serialización: `Move`: `type (u8) | direction (u8)`; `Pick` y `Take`:
  `type (u8) | direction (u8) | slot (u8)` (el casillero elegido: la
  selección del hotbar es local y viaja con la acción). `Sleep`:
  `type (u8) | accept (u8)`
  (1 = confirma dormir, 0 = cancela y sale de la cama). `GiftResponse`:
  `type (u8) | sender_id (u8) | accept (u8)` (1 = acepta el regalo).
  `ToggleWalking`: `type (u8)` sin payload. La tecla `X` produce este
  mensaje one-shot y alterna el modo server-side. La tecla es `Shift`
  (izquierdo o derecho).

---

## 2. Qué RECIBE el cliente — estado y eventos

Archivo: `common/common_server_status.h`. El servidor manda el **estado
completo** en cada lote: empieza con `Reset` (10), describe a todos los
jugadores, minerales, drops y slots ocupados, y cierra con `SyncEnd` (18).
El cliente acumula el lote entero y lo aplica recién al recibir `SyncEnd`
(sin ese cierre podría dibujar un frame entre el `Reset` y las entidades:
mundo vacío = el parpadeo). Lo que no vuelve en el lote ya no existe.

| Tipo | Mensaje | Payload | Significado |
|------|---------|---------|-------------|
| 2 | `Welcome` | `player_id u8` | Se envía una vez al conectarse |
| 1 | `Status` | `player_id u8, x u16, y u16, direction u8, appearance 8×u8, health u16, max_health u16, energy u16 (×100), max_energy u16 (×100), fainted u8` | Pose, personalización y vida/energía de un jugador. `fainted` = 1 cuando el jugador se desmayó por falta de energía o por agotar el horario del día (la vida se conserva): dispara la pose Faint y el popup no cancelable, igual que la muerte en combate. El cliente reacciona al flanco de `health` y de `fainted`: 0 = desmayo terminal (Faint + sonido, hasta que la vida/energía vuelvan); el salto a <20 % deja el charco de sangre en su celda (misma regla que `World::damage_player`) |
| 3 | `Leave` | `player_id u8` | Un jugador se desconectó |
| 4 | `Resource` | `x u16, y u16, content_id str, damage u8` | Mineral visible (`damage`: 0 intacto, 1 fisurado, 2 roto) |
| 5 | `Dropped` | `x u16, y u16, content_id str, interactive u8` | Item caído en el piso (`interactive`: 1 = se levanta con Take y anima la recogida, 0 = solo entra al inventario) |
| 6 | `InventorySlot` | `slot u8, count u8, content_id str, tradable u8` | Slot ocupado del inventario propio. `tradable` (0/1) = si se puede regalar (lo decide el server con `Item::can_be_traded`; con 0 el cliente ni ofrece el regalo) |
| 7 | `Obtained` | `content_id str` | Evento puntual: el jugador obtuvo un contenido |
| 8 | `ToolUsed` | `player_id u8, content_id str` | Un jugador usó su herramienta (animación de golpe); `content_id` es lo equipado (vacío = manos) |
| 9 | `ResourceHit` | `x u16, y u16, destroyed u8` | Evento puntual: un golpe a un mineral |
| 10 | `Reset` | (sin payload) | Primer mensaje de cada lote de estado: borra mapa e inventario del cliente |
| 11 | `Animal` | `id u8, kind str, x u16, y u16, direction u8, ready_to_milk u8` | Animal del mundo |
| 12 | `Sleep` | `sleeping u8, total u8` | Aviso de sueño dirigido a cada jugador. `sleeping > 0` = estoy en la cama (cantidad de durmientes); `total` = jugadores en la partida. Se manda solo al cambiar |
| 13 | `Furniture` | `x u16, y u16, content_id str` | Mueble del mundo (hoy la cama). El cliente todavía no lo dibuja |
| 14 | `Day` | (sin payload) | Amaneció: el día terminó por sueño o por timeout de 30 s |
| 15 | `SleepingPlayers` | `count u8, ids u8 × count` | Quiénes están en la cama (IDs). Va a todos los jugadores; se manda solo al cambiar |
| 16 | `Clock` | `day u8, minutes u16` | Día de juego (1 a 28, vuelve al 1) y hora: minutos desde las 6:00 (0 a 1200). La hora salta de a 10 min; se manda al cambiar, al amanecer (día nuevo, 0) y al entrar un jugador |
| 18 | `SyncEnd` | (sin payload) | Último mensaje de cada lote de estado: el cliente aplica el lote acumulado recién acá (evita el frame a medias = parpadeo) |
| 19 | `Crop` | `x u16, y u16, crop_id str, stage u8, needs_water u8, interactive u8, quality u8` | Cultivo plantado. `crop_id` = contentId del producto (`strawberry`, `onion`...); `stage` 0 semilla, 1..5 crecimiento, 6 cosecha; `needs_water` pide riego; `interactive` = cosechable con Take; `quality` 1..5 = BASE..PERFECT (misma línea que `qualities` de crops.toml; solo cambia la apariencia en la cosecha) |
| 20 | `GroundEffect` | `x u16, y u16, effect u8` | Consecuencia en la celda: `0` = arada (Dug), `1` = regada (Watered), `2` = charco de sangre (Blood). Una celda regada manda ambos, en ese orden. El charco lo manda el server con la regla de `World::damage_player` (golpe que dejó al jugador por debajo del 20 %, muerte incluida): es **estado persistente del día** (se seca al terminar el día o lo consume una vampira, `World::process_end_of_day_floor`/`consume_blood_puddle`) y va a **todos** los jugadores para que vean de dónde pueden revivir los vampiros |
| 21 | `Enemy` | `id u16, kind str, x u16, y u16, direction u8, health u8, action u8` | Enemigo del mundo. `kind` = `skeleton`, `orc`, `vampire`, `vampiress`, `skull` (vocabulario de `enemies.toml`; la vampira se dibuja como el vampiro). `health` 0 = muerto (pose de muerte). `action` (`EnemyAction`): 0 quieto, 1 ataque, 2 daño. El cliente anima el paso por interpolación y cuenta la ventana de la acción desde el cambio de `action` |
| 22 | `Projectile` | `id u16, kind str, x u16, y u16, direction u8` | Llama del skull en vuelo. `kind` = `skull_fire` o `skull_fire_green` (vocabulario de `enemies.toml`). Sin vida ni acción: vuela hasta la celda que ocupaba el jugador al lanzar, daña al contacto (5 / 10) y desaparece al chocar contra una pared, contra el borde o al llegar a su destino. Las llamas no bloquean celdas |
| 23 | `Chat` | `player_id u8, characterName str, text str, system u8` | Mensaje de chat. `system = 1` = mensaje del servidor (respuesta de cheat/error): se muestra sin prefijo de jugador; si no, prefijo `characterName`. Evento (no estado): se aplica inmediato, fuera del lote Reset…SyncEnd |
| 24 | `GiftRequest` | `sender_id u8, characterName str, content_id str` | El emisor (`sender_id`) ofreció su ítem equipado (`content_id`); el server lo reenvía al **receptor**. `characterName` = nombre del emisor para el popup (`{0}`). Evento (no estado): se aplica inmediato, fuera del lote; el receptor contesta con `GiftResponseMessage` (sección 1) |
| 25 | `GiftAnswered` | (sin payload) | El receptor contestó el pedido de regalo (acepte o rechace): el server lo manda **solo al emisor**, que así cierra su popup de espera. Evento |

| 34 | `PlayerName` | `player_id u8, characterName str` | Nombre público. Al entrar se reciben los nombres existentes y se transmite el del nuevo a todos, incluso en otros mapas. Se conserva entre lotes y se elimina con `Leave` |
| 35 | `InventoryFull` | (sin payload) | Evento privado de recogida rechazada por capacidad. Se traduce a `ViewNotice{"inventory_full"}` y usa el cartel real de SDL; la tecla I de demo fue retirada |

- `str` = `u16` de longitud + bytes.
- Enteros en big-endian (`htons`/`ntohs`).
- `content_id` es el vocabulario compartido de `inventory.toml` y
  `resources.toml`: `pickaxe, shovel, watering_can, sword, silver, coal, lapis, gold, rock`.
- `Obtained` solo se manda al jugador que obtuvo el contenido, y no en el
  inventario inicial.
- Las dimensiones del mundo **no viajan por la red**: el servidor usa el mismo
  tamaño que la escena del cliente (`bedroom`, 30x20).
- **Cadencia de enemigos.** El lote `Reset…SyncEnd` es el estado completo: el
  cliente poda en el cierre a los enemigos y proyectiles que el lote no volvió
  (por eso `Reset` ya no vacía `enemies_`: borrarlo en cada lote reiniciaría
  la ventana de animación). El servidor corre dos relojes: el lento
  (`CreatureTick`, 4 s) revive, deambula y hace que el skull lance llamas; el
  rápido (`EnemyTick`, 400 ms = el paso del jugador) mueve a quien está
  persiguiendo — el skull también, hasta su distancia de tiro `kShotDistance`,
  sin pegarle — y vuela las llamas. El `action` se limpia en cada tick rápido,
  así cada golpe viaja como flanco `Idle → Attack` y el cliente abre su
  ventana de animación (`actionCycleTicks`) sin que un `Idle` la corte.
  Cooldown de ataque: 1 golpe cada 10 ticks rápidos (4 s), el mismo daño por
  segundo de antes del reloj rápido.

---

## 3. Dónde se CONECTA — `game::GameClient`

Archivo: `client/game/GameClient.h`. Interfaz única entre el cliente y el
servidor: el ÚNICO punto por donde el cliente **envía** (`sendCommand`) y
**recibe** (`worldState`). Todo el cliente (loop, render) depende solo de
esta interfaz.

```cpp
class GameClient {
public:
    virtual void sendCommand(const protocol::ClientMessage&) = 0;  // ENVIAR
    virtual const LocalWorldView& worldState() const = 0;          // RECIBIR
};
```

- `NetworkGameClient` (`client/game/NetworkGameClient.*`) es la implementación
  que usa la app. Mantiene las colas del `ClientSender`/`ClientReceiver` y
  traduce cada `ServerMessage` al `LocalWorldView` (jugadores, minerales,
  items caídos, inventario, eventos `obtained`).
- `Application::createGameClient` es el ÚNICO lugar del cliente SDL que nombra
  una implementación concreta de `GameClient`. El loop y el render trabajan
  contra `GameClient&`.
- `MockGameClient` simula el servidor sin red. Se usa en los tests de la vista.
- `LocalWorldView` es la vista que dibuja el render: no es un contrato de red.

---

## 4. CÓMO corre ese intercambio — constant-rate loop + colas

El modelo de la cátedra, del lado de red:

```
    Receiver (cola thread-safe)  ◄──  hilo de red  ◄──  servidor
    Sender   (cola thread-safe)  ──►  hilo de red  ──►  servidor
```

Las colas y los hilos **no los hace el cliente SDL**. Del lado SDL, todo
ese intercambio ocurre **dentro del tick del constant-rate loop**
(`client/sdl/loop/GameLoop.{h,cpp}` + `FrameClock`, algoritmo del
artículo *Constant Rate Loop* — estrategia *keep working*, reloj
`std::chrono`). **No hay un `while` bloqueante aparte: ese while es este
loop**, y corre una vez por iteración (~60 por segundo):

```
cada iteración (número `it`):
  (1) ACÁ SE DRENA EL RECEIVER  → worldState() actualiza el estado
  (2) ACÁ SE MANDA AL SENDER    → sendCommand() pushea el comando
  (3) ACÁ SE PRESENTA           → motion.observe(vista, it)
                                  → cámara sigue el rect del jugador local
                                  → isWalking(): ¿se ve caminando?
                                  → ¿suena el paso? (FootstepCue)
                                  → renderer.drawWorld(..., motion, it, camera.viewport())
```

El paso (3) es presentación entera: **el sonido del paso vive ahí, no en
(2)**, porque sale del mismo cálculo que decide el asset (la fase
observada de este frame → `walkFrameIndexFor` → cuadro). Un solo criterio
por tick: el audio y el sprite cambian juntos, no pueden desfasarse.
`game::ViewMotion` (la historia de pasos observados) también vive en el
tick y se le pasa al render como dato.

`isWalking()` es el otro criterio que comparten render y sonido: el
servidor dice que la entidad se mueve, **o** el cuerpo todavía está
cerrando el paso que aceptó. Lo segundo existe porque al soltar la tecla
`moving` baja en el mismo tick pero el desplazamiento interpolado sigue
hasta terminar la celda: sin ese cierre, los pies se plantarían y el
cuerpo seguiría resbalando.

Los marcadores están en dos archivos: `client/core/Application.cpp` (el
tick, con el orden paso a paso) y `client/game/GameClient.h` (los dos
métodos por donde entra y sale la información).

Por qué dentro del tick y no en un while aparte:

- El pop es **no bloqueante**: si no hay mensajes se sigue con el último
  estado conocido y el juego sigue dibujando a tasa constante; si el
  servidor tarda, la pantalla **no se congela**.
- Hay **un** loop que coordina red, input y render; no dos compitiendo.
- `it` (el número de iteración) es el que sincroniza la animación: si el
  loop descarta iteraciones, el render saltea los cuadros que
  corresponden en vez de llevar su propia cuenta.

> El "main while" que arma el equipo de red (pop de Receiver → actualizar
> → push a Sender) es exactamente este ciclo; la diferencia es que acá
> vive en el tick del loop, no en un hilo o un while suelto.

Fuera de nuestro alcance (y a propósito): colas, hilos y serialización
los hace el equipo de red. El cliente solo llama a `worldState()` y
`sendCommand()`.

---

## Resumen

| Qué | Archivo | Estado |
|-----|---------|--------|
| Comando cliente→server | `common/common_command.h` | **implementado** (Move, Pick, Take, Sleep, Wake, Chat, GiftResponse) |
| Estado y eventos server→cliente | `common/common_server_status.h` | **implementado** (25 tipos, sync por diferencias) |
| Servidor autoritativo | `server/world_view.*`, `server/match.*` | **implementado** |
| Punto de conexión | `client/game/GameClient.h`, `NetworkGameClient` | **implementado**, usado por la app |
| Dónde se drena/pushea | `client/sdl/loop/GameLoop.*` + tick en `client/core/Application.cpp` | listo: constant-rate loop (sección 4) |

Ambos lados compilan la **misma** definición de `common/`.
