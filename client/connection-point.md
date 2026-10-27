# Punto de conexión cliente ↔ servidor

Documento para coordinar con quien hace **protocolos**. Resume qué manda el
cliente, qué necesita recibir, y **dónde exactamente** se conecta la red.

El cliente es **dummy** (ver `Semana 2.md`): no decide la lógica de juego.
Manda *intenciones* y dibuja el *estado* que el servidor devuelve. Del lado
del cliente **solo hacemos SDL**: la implementación de red de `GameClient`
la hace el equipo de red/servidor; nosotros dejamos el punto de conexión y
el contrato listos.

```
InputManager ──► Command ──► GameClient (interfaz = punto de conexión)
                              │  sendCommand(cmd)   [cliente → servidor: ENVIAR]
                              │  worldState()       [servidor → cliente: RECIBIR]
                              ▼
     MockGameClient  (lo que corre HOY: simula al server, sin red)
     (mañana: una implementación de red de GameClient, que hace el
      equipo de servidor; el cliente SDL no cambia)
```

---

## 1. Qué MANDA el cliente — `protocol::Command`  (ACORDADO)

Archivo: `common/common_command.h`. El cliente arma un `Command` con la
acción del jugador y lo envía; el servidor valida y responde.

```cpp
enum class Move { Walk, Pick };

struct Command {
    Move         move;       // Walk = caminar, Pick = picar/interactuar
    std::uint8_t x;          // CELDA DESTINO (absoluta) que PROPONE el cliente
    std::uint8_t y;
    Direction    direction;  // hacia dónde mira/va
};
```

- Para el **personaje**, el cliente **propone la celda destino** `(x, y)`;
  el servidor decide si es válida (pared, colisión) y responde con el
  estado resultante.
- `x/y` son celdas **absolutas** de la grilla lógica (0..255 por eje).
- `Direction` (`common/common_game_direction.h`): `Front, Back, Left,
  Right` (`Front` = hacia abajo).

**A acordar con protocolos:** formato de serialización de `Command` (orden
de campos, endianness). El transporte ya existe: `common/Protocol` (bloques
con prefijo de tamaño).

---

## 2. Qué NECESITA RECIBIR el cliente — estado del mundo  (A DEFINIR)

Todavía **no está acordado** el mensaje de estado que el servidor manda al
cliente (posiciones de entidades, inventario, etc.). **Falta definirlo con
protocolos** y, cuando se defina, vivirá en `common/` como contrato
compartido.

Mientras tanto, el cliente dibuja a partir de una **vista local
provisional** (`client/game/LocalWorldView.h`), que NO es un contrato de
red: es solo "lo que la presentación necesita hoy". Hoy la llena el mock con el
jugador, los animales de su simulación, los ítems del suelo que esos
animales sueltan (ej. huevos: ver `animals-entry-point.md`), los
**eventos de sonido del tick** (cacareos: `soundEvents`, ver abajo) y el
tamaño de su mundo (`grid`). Su forma (una lista de entidades con celda +
dirección + si se mueve, una lista de ítems del suelo con celda + ítem,
una lista de eventos de sonido, más las dimensiones de la grilla) sirve
de referencia de qué necesita el cliente para dibujar y sonar.

**Las dimensiones del mundo también son estado del servidor.** `LocalWorldView`
lleva `grid` (`columns`/`rows`); el mock lo completa con el tamaño de su
mundo simulado y, cuando exista el DTO, la implementación de red lo llenará
con las dimensiones que mande el servidor. El servidor manda **solo el
tamaño** y las posiciones en celdas; el cliente **deriva el layout** de la
sala (piso, paredes) a partir de ese tamaño — el servidor **nunca** manda
sprites ni layout.

**A acordar con protocolos (cuando se defina el estado):**
- **Unidades de posición:** el cliente las quiere en **celdas lógicas** (las
  traduce a píxeles con `ScenePixelMapper`), para desacoplar al servidor de
  la resolución.
- Qué entidades incluye (jugador, otros jugadores, gallinas, vacas) e
  **id estable** para seguirlas entre frames. La vista local YA lleva ese
  campo: `ViewEntity.id` (`int`, el jugador es `0`, los animales del mock son
  `1..N`) — cuando se defina el estado, la implementación de red llena ese
  campo con el id que mande el servidor.
- Los **ítems en el suelo** (ej. huevos que ponen los animales): la vista
  provisional ya los lleva (`groundItems`, cada uno con id estable + celda +
  `ItemKind`) para que el DTO los incluya; el server también define las
  reglas de recogida, tope y despawn (hoy el mock topa en 3, silencioso).
- Los **eventos de sonido** (cacareos y mañana otros): la vista provisional
  ya los lleva (`soundEvents`, cada uno con cue + id del emisor) y son
  **per-tick** — duran un tick, no se acumulan. Acordar si viajan en el
  mismo mensaje o por otro (son perfectos para deltas/UDP: perder uno no
  rompe nada); el server también define el **filtrado por relevancia**
  (hoy el mock emite siempre y el cliente silencia lo que su sala no
  declara en `scenes.toml`; el server real puede filtrar antes de mandar).
- Si el **inventario** viaja en ese mensaje o por otro.
- **Frecuencia / formato** (estado completo por tick vs. deltas).
- **Dimensiones del mundo:** `columns`/`rows` de la grilla lógica, para que
  el cliente derive el layout de la sala.

---

## 3. Dónde se CONECTA — `game::GameClient`

Archivo: `client/game/GameClient.h`. Interfaz única entre el cliente y el
servidor: el ÚNICO punto por donde el cliente **envía** (`sendCommand`) y
**recibe** (`worldState`). Todo el cliente (loop, render) depende solo de
esta interfaz.

```cpp
class GameClient {
public:
    virtual ~GameClient() = default;
    virtual void sendCommand(const protocol::Command&) = 0;  // ENVIAR
    virtual const LocalWorldView& worldState() const = 0;    // RECIBIR
    virtual void update(int iteration) = 0;                  // TICK de simulación
};
```

`update(it)` es el hook del tick: **UNA llamada por iteración del loop, al
inicio del tick** (sección 4). Hoy avanza la simulación de animales del
mock; mañana, la implementación de red drena acá su Receiver (hace el pop
no bloqueante de la cola y actualiza el estado que devolverá
`worldState()`). Así el intercambio completo de red vive en un solo lugar
del tick.

- Hoy corre **`MockGameClient`** (`client/game/MockGameClient.*`): simula al
  servidor sin red (lleva al jugador a la celda propuesta con clamp a la
  grilla, arma la vista). Es lo que permite avanzar el cliente sin servidor.
- **La implementación de red** la hace el equipo de servidor: otra clase que
  implemente `GameClient`, serializando el `Command` al enviar y
  deserializando el estado al recibir (sobre `common/Protocol`). **No está
  esbozada acá a propósito** (es trabajo fuera de SDL).
- **Integrar = cambiar una línea** en `Application::createGameClient`
  (devolver esa implementación en vez del mock). `createGameClient` es el
  ÚNICO lugar del cliente SDL que nombra una implementación concreta de
  `GameClient`; el loop (`runGameLoop`) y el render trabajan contra la
  interfaz `GameClient&`, así que no se tocan.
- Cuando se acuerde el estado de red (sección 2), el retorno de
  `worldState()` pasará de `LocalWorldView` a ese contrato compartido. Las
  **dimensiones ya viajan en `LocalWorldView.grid`**: ese es el campo que la
  implementación de red completará con lo que mande el servidor.

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
  (1) TICK DE SIMULACIÓN      → gameClient.update(it): hoy avanza los
                               animales del mock; mañana, acá drena el
                               Receiver y worldState() queda actualizada
  (2) ACÁ SE MANDA AL SENDER  → sendCommand() pushea el comando
  (3) ACÁ SE PRESENTA         → motion.observe(vista, it)
                                → isWalking(): ¿se ve caminando?
                                → ¿suena el paso? (FootstepCue)
                                → renderer.render(..., motion, it)
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
tick, con el orden paso a paso: la primera línea del tick es
`gameClient.update(it)`) y `client/game/GameClient.h` (los tres métodos por
donde entra, sale y late la información).

Por qué dentro del tick y no en un while aparte:

- El pop es **no bloqueante**: si no hay mensajes se sigue con el último
  estado conocido y el juego sigue dibujando a tasa constante; si el
  servidor tarda, la pantalla **no se congela**. Hoy el mock lo replica:
  `update(it)` es no bloqueante y deja el último estado conocido.
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

## Resumen para la reunión

| Qué | Archivo | Estado |
|-----|---------|--------|
| Comando cliente→server | `common/common_command.h` | **acordado**; falta serialización |
| Estado server→cliente | (a crear en `common/`) | **a definir**: unidades (celdas), entidades, inventario, deltas, dimensiones (`columns`/`rows`) |
| Punto de conexión | `client/game/GameClient.h` | listo (enviar/recibir); la implementación de red la hace servidor |
| Dónde se drena/pushea | `client/sdl/loop/GameLoop.*` + tick en `client/core/Application.cpp` | listo: constant-rate loop, seam marcado (sección 4) |

El `Command` ya está en `common/`, así que **ambos lados lo compilan y usan
la misma definición**.
