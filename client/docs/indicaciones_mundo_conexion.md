# Indicaciones para la conexión de mundos servidor ↔ cliente

> Para quien conecte la carga de mundos (`worlds/<nombre>.toml`) entre el
> servidor y los clientes (lobby Qt + SDL). El cliente ya carga cualquier
> mundo del editor usando `world::WorldMapLoader` + `world::MapSceneAdapter`
> (`world::SceneSources`) y sigue a `game::LocalWorldView::mapId` mediante
> `world::SceneSwitcher`. Acá está lo que falta implementar en el servidor
> y los stand-ins provisorios que hay que limpiar del cliente al conectarlo.

## 1. Reparto de responsabilidades

- **El servidor es dueño de la verdad del mundo y del mapa activo**:
  - Lista los mundos válidos de `worlds/` para ofrecerlos en el lobby.
  - Carga el `worldmodel::World` elegido al crear una partida.
  - Fija el cupo máximo de la partida según la cantidad de cabañas, asigna
    una cabaña libre (`cabin_1`..`cabin_N`) a cada jugador y define su
    spawn y su cama.
  - Aplica las colisiones de cada mapa, ubica los comerciantes y ejecuta
    los portales entre mapas y entradas por puerta de edificios.
  - Informa a cada cliente el `worldName` de la partida al entrar y su
    `mapId` actual (`LocalWorldView::mapId`) al spawnear y cada vez que
    cambia de mapa.
- **El cliente SDL es dueño solo de la presentación**:
  - `Application::runGame` resuelve `worlds/<worldName>.toml` desde
    `core::kWorldsDir` (si `session.worldName` viene informado) y pasa esa
    ruta a `PlaySession`.
  - `PlaySession` carga el mundo junto con `overlay.toml` y `scenes.toml`
    mediante `world::SceneSources` (`WorldMapLoader` + `MapSceneAdapter`).
  - Cuando llega `LocalWorldView::mapId` con un mapa válido, `PlaySession`
    dispara `SceneTransition::changeTo` a través de `SceneSwitcher` (si
    `mapId` llega vacío, hoy queda en `core::kInitialRoom` como fallback).
  - Si el servidor llena `LocalWorldView::npcPlacements`, pisa las celdas
    por defecto tomadas de los `Marker` de cada mapa.

## 2. Qué falta implementar en el servidor

### CMake (`server/CMakeLists.txt`)

- Linkear `taller_world_model` a `taller_server`.
- Definir `TALLER_WORLDS_DIR="${CMAKE_SOURCE_DIR}/worlds"` en las
  definiciones de compilación de `taller_server` (mismo criterio que
  `client/CMakeLists.txt`).

### Protocolo de lobby

- Agregar un pedido de lobby para listar los mundos válidos de
  `TALLER_WORLDS_DIR` usando `worldmodel::WorldRepository` y
  `worldmodel::WorldValidator`, devolviendo la lista de nombres
  (`WorldSummary{name, ""}`).
- Recibir `worldName` en `MatchRequest` cuando `type == MatchRequestType::Create`.
- Devolver el `worldName` de la partida al cliente en `ServerWelcome`
  (tanto en `Create` como en `Join`), para que ambos clientes carguen el
  mismo archivo `worlds/<worldName>.toml`.

### Cupo y asignación de cabañas (`1..4`)

- Contar los objetos con `role == worldmodel::ObjectRole::Cabin` en
  `world.maps.at("farm").objects` para fijar el máximo de jugadores de la
  partida en `MatchesMonitor`.
- Asignar una cabaña libre `k` (`1..N`) a cada jugador que entra a la
  partida.
- Usar el marcador `worldmodel::MarkerKind::Bed` de
  `world.maps.at("cabin_" + std::to_string(k))` como punto de spawn y
  posición de cama de ese jugador (tanto al entrar como al despertar en
  un nuevo día).
- **Cómo se dispara el sueño en la cama (`MarkerKind::Bed`)**: el servidor
  debe disparar la consulta de sueño (`ServerSleep`) cuando el jugador pisa
  o interactúa en la celda `MarkerKind::Bed` de su cabaña. (En el cliente,
  si el `overlay.toml` configura `enter` sobre la cama, usa el mismo marker
  `bed`, y en `dayEnded` hay que respetar `mapId` en lugar de volver fijo
  a `kInitialRoom`).

### Colisión, comerciantes y portales

- Cargar `map.collision[row * map.width + col]` en `Cell::is_wall` para
  cada mapa cargado del mundo.
- Ubicar a cada comerciante según el marcador de su respectivo mapa
  interior obligatorio (y enviarlo en `npcPlacements` si corresponde):
  - `world.maps.at("carpentry").markers` → `MarkerKind::MerchantRobin` (`"robin"`)
  - `world.maps.at("blacksmith").markers` → `MarkerKind::MerchantClint` (`"clint"`)
  - `world.maps.at("general_store").markers` → `MarkerKind::MerchantPierre` (`"pierre"`)
  - `world.maps.at("ranch").markers` → `MarkerKind::MerchantMarnie` (`"marnie"`)
- Además de los `portals` explícitos de cada mapa, en `"farm"` y `"town"`
  las celdas de puerta de los edificios con interior (`obj.hitsDoor(pos)`,
  que contempla `doorWidth > 1` como la puerta doble de `tienda_de_pierre`)
  permiten entrar a su respectivo interior:
  - `ObjectRole::Cabin` → `"cabin_k"` (según el índice de cabaña en `"farm"`)
  - `ObjectRole::Carpentry` → `"carpentry"`
  - `ObjectRole::Blacksmith` → `"blacksmith"`
  - `ObjectRole::GeneralStore` → `"general_store"`
  - `ObjectRole::Ranch` → `"ranch"`
  - (`ObjectRole::CommunityCenter` en `"town"` no tiene puerta ni mapa interior).
- Cuando un jugador pisa una celda dentro del rectángulo de un
  `worldmodel::Portal` (`cell`, `width`, `height`), moverlo al mapa
  `portal.targetMap` en la celda `portal.targetCell`.
- Agregar el mensaje de red correspondiente y enviar al cliente el `mapId`
  actual del jugador (al entrar a la partida y en cada transición por
  portal, puerta o despertar en la cabaña) para que `NetworkGameClient`
  actualice `LocalWorldView::mapId`.
  *(Nota: `ServerMessageType` ya usa los valores `23`, `24` y `25` para
  `Chat`, `GiftRequest` y `GiftAnswered`; usar un id nuevo, p. ej. `26`.)*

## 3. Lo que hay que BORRAR o ajustar en el cliente cuando el servidor tome el control

Una vez que el servidor envíe la lista de mundos, `worldName` al entrar y
el `mapId` actual del jugador en partida:

1. **`client/qt/services/LocalWorldService.h` y `client/qt/services/LocalWorldService.cpp`**:
   - Eliminar ambos archivos, quitarlos de `client/CMakeLists.txt` y
     quitar su `#include` y uso en `client/qt/LobbyWindow.cpp`.
   - Reemplazar `LocalWorldService` por el servicio de red que pide la
     lista de mundos al servidor.
2. **`client/qt/LobbyWindow.cpp` (`LobbyWindow::onCharacterConfirmed`) y `client/qt/services/NetworkSessionService.cpp` (`createGame` y `runRequest`)**:
   - En `LobbyWindow::onCharacterConfirmed`: borrar el bloque
     `// shortcut:` que toma el primer mundo local de `worldService_->worlds()`
     cuando `session.worldName` viene vacío en `Join`.
   - En `NetworkSessionService::createGame`: borrar el `// shortcut:` y
     enviar `selection.worldName` en `MatchRequest`.
   - En `NetworkSessionService::runRequest`: borrar el `// shortcut:` que
     emite `emit gameJoined(request.codeMatch, "");` y pasar el
     `worldName` real devuelto por el servidor.
3. **`client/core/PlaySession.cpp` (`dayEnded`, `CycleMap`, `ToggleMarket`, `enterStructureMap`)**:
   - En `dayEnded`, usar el `mapId` que mande el servidor (o el spawn de
     su cabaña) en lugar de forzar `core::kInitialRoom`.
   - Cuando el protocolo soporte entrar por edificio/puerta, hacer que
     `enterStructureMap` (o el caminar sobre puertas/portales) delegue al
     servidor en vez de cambiar la escena localmente, y limpiar las teclas
     de prueba (`CycleMap`, `ToggleMarket`).
4. **`client/core/Constants.h`, `client/core/Application.cpp` y `scenes.toml`**:
   - Cuando todas las partidas carguen siempre desde `worlds/<worldName>.toml`,
     eliminar el fallback a `scenes.toml` (`kScenesFile`, `kWorldFile`,
     `kInitialRoom`, `kMarketRoom`) y esperar `LocalWorldView::mapId`
     en lugar de arrancar en `"bedroom"`.
