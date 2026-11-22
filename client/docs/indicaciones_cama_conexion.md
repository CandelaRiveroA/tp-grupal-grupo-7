# Indicaciones para la conexión servidor ↔ cliente

> Para quien conecte el server con el cliente (SDL). Todo lo de la cama,
> los popups y las transiciones de escena quedó armado como
> **presentación**: SDL dibuja lo que la vista (`game::LocalWorldView`)
> dice y expone unos **puntos de entrada** para que la fuente de estado
> decida qué pasa. Acá está todo lo que hay que saber para enchufarlo.

## 1. De quién es qué

- **El server es dueño de "qué pasa"**: posiciones, minerales, inventario,
  cuándo se pregunta, cuándo empieza un nuevo día, cuándo cambia la escena.
- **SDL es dueño de "cómo se ve y suena"**: dibuja lo que recibe por la
  vista y reproduce los cues que la fuente emite. No inventa ni borra
  estado: si el server manda los minerales de nuevo, se ven; si manda al
  jugador en la celda `lie` de la cama, salen las frazadas + la pose solas.

## 2. Los puntos de entrada que SDL ya ofrece

Los llama la fuente de estado; hoy los disparan atajos de prueba (ver §4).

| Método | Qué hace |
|---|---|
| `game::PopupState::showQuestion()` | Muestra la pregunta "¿Terminamos el día?" (popup modal con Sí/No) |
| `game::PopupState::showWaiting(ready, total)` | Muestra "Esperando al resto de jugadores... (ready/total)" con el botón de cancelar |
| `game::PopupState::hide()` | Cierra el popup |
| `game::SceneTransition::changeTo(scene)` | La transición "apagado y volver a prender" al escenario indicado (`"bedroom"`, `"cabin_1"`, `"farm"`, ...). Una a la vez: si llega otra mientras corre, se ignora |
| `LocalWorldView::soundEvents` | Los cues que emite la fuente y SDL reproduce (p. ej. el gallo al empezar el día) |

Viven en objetos que instancia `core::PlaySession` (`popupState_`,
`sceneTransition_`); para llamarlos desde la conexión se enganchan igual
que el flujo actual de la vista.

> **Estado de la conexión:** conectado. `NetworkGameClient` deja
> `ServerSleep` y `ServerDay` en la vista (`LocalWorldView::sleep` y
> `LocalWorldView::dayChanged`), y `PlaySession` los traduce a los métodos
> de arriba. Los votos (Sí/No) y la cancelación salen como `SleepMessage`.
> Los stand-ins de prueba ya se borraron (§4).

## 3. El flujo del "dormir" (lo que se espera del server)

1. El jugador apreta la cama → **intención de dormir** (mañana: un comando
   por la conexión; hoy el click local es el stand-in).
2. El server valida (posición, reglas) y responde llamando
   `showQuestion()`.
3. El jugador vota **Sí / No** → la intención viaja al server (hoy, a
   nivel dibujo, solo cierra el popup).
4. El server decide:
   - **Quedan jugadores activos** → `showWaiting(hechos, total)`. El
     popup se cierra con el **botón de cancelar** (esa cancelación también
     es una intención hacia el server).
   - **Todos durmieron → fin del día** → `ServerDayEnd`: la pantalla queda
     en negro con el popup de OK (sin cancelar) y el reloj queda congelado.
     Cada cliente manda `WakeMessage` al apretar OK.
   - **Despertaron todos** → `ServerDay`: `changeTo(...)` a la cabaña del
     jugador (`mapId`, p. ej. `"cabin_1"`..`"cabin_N"`, o `"bedroom"` en el
     fallback de `scenes.toml`) **y el cue del gallo**. El reloj vuelve a
     las 6:00 y todos quedan en su cama (`MarkerKind::Bed`). (Hoy en
     `PlaySession` el `dayEnded` sigue usando `kInitialRoom` hasta que el
     servidor informe `LocalWorldView::mapId`.)

### El jugador muerto no se le pregunta

Al morir (vida en 0 o energía agotada) el server lo registra en la cama
como aceptado y empuja el estado de sueño **en el mismo tick de la
muerte** (`Match::handle(EnemyTick)`): sin eso el muerto se enteraba
recién cuando mandaba un comando, es decir, cuando se movía.

Del lado SDL, `PopupState::showSleepState(sleeping, total, voted,
localDied)` manda al muerto directo a `showWaiting` **sin la pregunta y
sin el botón de cancelar** (`waitingCancelable` apagado con la vida del
jugador local). Espera ahí hasta que el resto vote o muera también.

### El "reinicio del día" NO arma nada

`changeTo(...)` solo cambia **qué se pinta**: se cargan los datos de la
escena, se reconstruye el índice estático y la cámara. La ventana, el game
loop, la sesión y la conexión **siguen intactos**. Todo lo demás (minerales,
entidades, inventario, posición del personaje) lo sigue mandando el server
por la vista, como siempre.

## 4. Lo que hay que BORRAR cuando el server tome el control

Hecho: se borraron los stand-ins.

1. `client/game/InputCommand.h` — sin `Kind::TestWaiting` ni `Kind::TestSceneChange`.
2. `client/sdl/input/InputManager.cpp` — sin los bindings `SDLK_y` ni `SDLK_b`.
3. `client/core/PlaySession.cpp` — sin los bloques de prueba ni el
   `showQuestion()` local del click en la cama. El popup lo abre `ServerSleep`.

El mecanismo (popups, transición, fades, re-indexado) no se tocó.

## 5. Reglas del lado SDL que conviene saber

- **Los popups son modales**: mientras se muestra uno, no hay input del
  juego hasta responder (Sí/No) o cerrar (cancelar).
- **La transición bloquea el input** durante el fade; el cambio de datos
  ocurre exactamente en el negro (se puede mandar todo el estado nuevo
  tranquilo).
- **La posición manda**: la cama (frazadas + pose mirando a un lado) se
  derivan de la vista; no hay que avisar nada especial.
- **Las voces del server** sobre popups/escena = los métodos de §2; el
  dibujo no se toca al conectar (mismo criterio que el resto del cliente).

## 6. Dónde está cada cosa (mapa rápido)

| Qué | Dónde |
|---|---|
| Estado del popup + puntos de entrada | `client/game/PopupState.{h,cpp}` |
| Transición de escena + `changeTo` | `client/game/SceneTransition.{h,cpp}` |
| La cama (entrar, pose, frazadas) | `client/game/EnterSelection.{h,cpp}` + `enter` en `scenes.toml` u `overlay.toml` (`MapObjectAdapter` usando `MarkerKind::Bed`) |
| Texto con fuente (reutilizable) | `client/sdl/render/TextRenderer.{h,cpp}` + `ui/text.toml` |
| Figura del popup (grilla + marco) | `client/world/InventoryLayout` + `client/sdl/render/InventoryHud` |
| El swap de escena (en el negro) | `core::PlaySession::changeScene` |

> Para la conexión completa de mapas del editor (`worlds/<nombre>.toml`,
> `LocalWorldView::mapId`, `MarkerKind::Bed` como spawn y cabañas `1..N`),
> ver `client/docs/indicaciones_mundo_conexion.md` y `client/docs/convenciones.md` (§14).
