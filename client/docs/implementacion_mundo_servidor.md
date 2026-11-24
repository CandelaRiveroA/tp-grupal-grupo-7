# Implementación: mundo del servidor como fuente de verdad

## Resumen

El servidor ahora carga `worlds/<worldName>.toml` (editor) al crear una partida y
construye cada escena desde él (tamaño, colisiones, portales, camas). El `mapId`
actual del jugador viaja en cada lote de estado para que SDL renderice la escena
correcta (`SceneSwitcher` ya seguía `LocalWorldView::mapId`). Los portales del
TOML ya transfieren al jugador entre escenas.

El cliente prefiere el `worldName` informado por el servidor para resolver
`worlds/<worldName>.toml`, pero conserva el fallback a `scenes.toml`/
`LocalWorldService` mientras no se complete la limpieza del cliente (ver
"Sigue pendiente").

La selección de mundos del lobby consulta al servidor (`ListWorlds`): el server
filtra con `WorldValidator` y devuelve solo los mundos válidos
(`MatchesMonitor::validWorldNames`). Al crear se envía el mundo seleccionado y
al unirse el cliente recibe el nombre real en `ServerWelcome`.

Ante un desajuste entre el mundo anunciado por el servidor y los archivos
locales, el cliente termina con error (solo log, sin diálogo).

Verificación: build completo + 402 tests verdes.

## Cambios principales

### CMake
- `CMakeLists.txt`: `taller_server` linkea `taller_world_model`; `TALLER_WORLDS_DIR`
  definido para tests.
- `server/CMakeLists.txt`: `TALLER_WORLDS_DIR` para el server.

### Protocolo (`common/`)
- `MatchRequest`: campo `worldName` (Create lo envía; Join lo ignora).
- `MatchRequestType::ListWorlds`: pedido de catálogo en una conexión de lobby
  independiente; la respuesta contiene la cantidad y los nombres de mundos.
- `ServerWelcome`: campo `worldName` (eco para el joiner).
- `ServerMapChange { mapId }` (id 31) + sender/receiver/variante.

### Server
- `SceneId` pasa de `enum class` a `using SceneId = std::string` (alias). Identidad
  única = id del TOML; la lógica de mazmorras usa `MapRules::dungeonLevel`.
- `World::World(const worldmodel::World&)`: una escena por mapa según `MapKind`
  (Farm/Town/Dungeon/Scene), tamaño por mapa, colisiones desde `map.collision`,
  portales (rect -> una entidad `Portal` por celda), `MarkerKind::Bed -> Bed`.
- `World(uint16_t,uint16_t)` se conserva solo para tests: lo usan numerosos
  tests unitarios y no tiene consumidores de producción.
- `Player` sin home explícito usa como hogar su escena y posición inicial; la
  partida sigue pasando explícitamente la cama de la cabaña asignada.
- `Match`: ctor recibe `worldmodel::World&&`; spawn desde `world.spawn_map/x/y`;
  se borran los portales hardcodeados (ahora vienen del TOML).
- `MatchesMonitor::create(code, worldName, ...)`: valida nombre, carga y valida con
  `WorldRepository`/`WorldValidator` (`Invalid` si falla); `MatchSeat.worldName`.
- `MatchesMonitor::validWorldNames()`: arma el catálogo filtrando nombres
  inválidos, archivos corruptos y mundos que no pasan `WorldValidator`.
- `client_handler`: pasa `request.worldName` y manda `ServerWelcome{..., worldName}`.
- `WorldView::sync`: agrega `ServerMapChange{escena del jugador}` tras `ServerReset`.

### Cliente (mínimo)
- `NetworkSessionService::createGame`: envía `selection.worldName`.
- `NetworkWorldService`: pide el catálogo por TCP de forma asíncrona y sustituye
  `LocalWorldService`; los errores de red y catálogo vacío se muestran en el lobby.
- `NetworkGameClient`: lee `worldName` del welcome + accessor; `apply(ServerMapChange)`
  actualiza `view_.mapId`.
- `Application::runGame`: prefiere el `worldName` del servidor para resolver
  `worlds/<name>.toml`, carga el mapa inicial desde el mundo y valida que tenga
  mapas.
- `SceneSources`: deshabilita la fuente de `scenes.toml` cuando recibe una ruta
  vacía, evitando mezclar mapas legacy con el mundo de la partida.
- `Application::runGame`: ante un fallo al resolver o leer el mundo, además del
  log muestra un diálogo con una descripción legible y el path problemático.

### Tests
- Sed de `SceneId::` en 7 archivos de tests (valores string).
- `matches_monitor_test.cpp`: prueba que el catálogo incluya mundos válidos.
- `world_test.cpp`: prueba que el hogar por defecto use la escena y posición inicial.
- `WorldTest.MundoAdCargaEscenasYTransfierePorPortal`: carga `worlds/ad.toml` y
  verifica transferencia `farm -> town`.

## Lo que quedó fuera (próximos pasos del plan)

### Hecho en esta pasada
1. **Puertas de edificios -> interiores**: el ctor TOML de `World` sintetiza un
   `Portal` por celda de puerta (`door`/`doorWidth`) de cada objeto con interior
   en `farm`/`town`; el destino se resuelve por `MapRules` (rol -> mapa cuyo
   `interiorObject` coincide; `Cabin` -> `cabinId(k)` por índice) y la celda de
   llegada es el `InteriorExit` del interior. El cliente acerca al jugador a la
   puerta con la misma auto-caminata de la cama (`player_.walkTo(structure.door)`);
   el cambio de escena lo decide el server vía `ServerMapChange`. `// shortcut:
   caminado naive sin pathfinding`.
2. **Comerciantes por marker**: los merchants ahora se colocan en su interior
   desde su marker (`MarkerKind::Merchant*` -> factory en `Town::make_merchant`);
   el storage/lookup se movió de `Town` a `Scene` base, y `World::get_nearest_merchant`
   busca en la escena actual del jugador. `Town::place_default_merchants` se mantiene
   solo para el ctor de tests `World(w,h)`.
3. **Animales**: `Match::place_animals` ajusta cada coords hardcodeada a la celda
   caminable más cercana del mapa de `farm`. `// shortcut: posición provisional;
   cuando haya marker de animales en el editor, generarlos desde ahí.`
4. **Cupo por cabañas**: `MatchesMonitor::create` fija la capacidad de la partida a la
   cantidad de objetos `role == Cabin` en `farm` (inyectable por dir de mundos);
   `Match` asigna una cabaña libre (`cabin_k`) a cada jugador que entra y su
   spawn/cama = `MarkerKind::Bed` de esa cabaña. `Player` ya tenía `home_*`.
   `joinLocked` revisa nombre de personaje antes que capacidad (más específico).
5. **Limpieza mínima del cliente**: borradas las teclas demo `CycleMap`/`ToggleMarket`,
   `dayEnded` ya no fuerza `kInitialRoom` (sigue el `mapId` del server), y borrados
   los shortcuts obsoletos de `LobbyWindow`/`NetworkSessionService`.
6. **Validación**: test `WorldTest.TodosLosMundosDelRepoSonValidos` valida con
   `WorldValidator` cada mundo de `TALLER_WORLDS_DIR` (hoy `ad.toml`).
7. **Carga inicial del cliente**: `Application` carga el mundo recibido en el
   welcome, adapta `farm` como mapa inicial y pasa su id a `PlaySession`;
   desapareció el fallback a `scenes.toml` y se eliminaron las constantes
   `kScenesFile`, `kWorldFile`, `kInitialRoom` y `kMarketRoom`.
8. **Catálogo remoto de mundos**: agregado el pedido `ListWorlds`, filtrado en
   el servidor con `WorldValidator` y servicio de red asíncrono en el lobby.
9. **Home de `Player`**: cuando no se especifica, queda asociado a la escena y
   coordenadas iniciales, evitando transferencias a `"farmhouse"` en mundos TOML.
10. **Error de mundo visible**: `Application` presenta un diálogo descriptivo
    cuando no puede resolver o cargar el mundo anunciado por el servidor.

### Sigue pendiente
- **Validación de mundos del editor**: el editor ya bloquea el guardado con
  `WorldValidator` (misma instancia que el server) y muestra los issues; el
  usuario coloca los portales a mano a su gusto (decisión de diseño, no se
  auto-generan). Lo pendiente es solo verificar manualmente más mundos exportados
  por el editor además de `ad.toml`/`ad2.toml`.

## Iteración de revisión (senior review)

Tras la implementación se hizo una revisión estilo senior sobre los cambios no
commiteados. Hallazgos y su resolución:

### Arreglados
- **Merchants por escena (chat/mercado roto en partidas reales)**:
  `World::get_merchant_for_command` todavía exigía `scene == "town"`, pero los
  mercaderes ahora viven en los interiores y `Town` ya no los coloca por
  defecto. Se borró el guard; el fallback `get_nearest_merchant` ya era por
  escena actual. Archivo: `server/domain/map/world.cpp`.
- **Asignación de cabaña monótona y sin liberación al salir**:
  `Match` llevaba `next_cabin_` que solo incrementaba; `leave()` decrementaba
  `players` pero no devolvía la cabaña. Ahora hay un `std::set<uint8_t>
  free_cabins_` y un `std::map<uint8_t,uint8_t> player_cabin_`: al entrar se
  toma la menor libre, al salir (PlayerLeft) se devuelve. Archivos:
  `server/match.h`, `server/match.cpp`.
- **Capacidad por cabañas sin test positivo**: se agregó
  `JoinDevuelveOkConSegundoJugadorEnMundoDeDosCabinas` (usa `worlds/ad2.toml`,
  2 cabañas) y `CabinCountYBedEnMundoAd`; `ad2.toml` se commitea force-added
  (igual que `ad.toml`, por el `.gitignore worlds/*`). Archivos:
  `tests/matches_monitor_test.cpp`, `tests/world_test.cpp`, `worlds/ad2.toml`.
- **`scene.cpp` sin `#include <algorithm>`**: `on_entity_removed` usaba
  `std::remove` solo por inclusión transitiva. Agregado. Archivo:
  `server/domain/map/scene.cpp`.
- **`MatchesMonitor::create` retenía el lock del lobby durante toda la carga/
  validación/copia del mundo** (~cientos de ms para `ad.toml`). Ahora carga,
  valida y calcula capacidad fuera del lock; bloquea solo para insertar en
  `matches`. Archivo: `server/matches_monitor.cpp`.
- **`enterStructureMap` muerto**: `structureActions_` ya no mapeaba `EnterMap`
  y `runStructureAction` maneja la puerta directamente; se borraron la
  declaración y la definición. Archivos: `client/core/PlaySession.h`,
  `client/core/PlaySession.cpp`.
- **Includes muertos en `matches_monitor_test.cpp`**: se borraron 8 headers
  que ya no se usaban tras quitar el fixture de 4 cabañas.

### Aceptados / pendientes (no se tocaron)
- **`WorldTest.TodosLosMundosDelRepoSonValidos` solo imprime el nombre en el
  fallo**: no incluye el issue. Diagnóstico pobre pero no rompe nada.
- **Animales desde datos/marcadores**: el snap a celda caminable es provisional;
  la solución definitiva eliminar el spawn
  inicial, los animales solo se obtienen comprando a Marnie.
- **Portales `farm<->town`/`farm<->dungeon` en mundos del editor**: el usuario
  los coloca a mano en el editor; `WorldFactory::wirePortals` solo arregla los
  `InteriorExit` de edificios.

## Riesgos conocidos

- `World(uint16_t,uint16_t)` se mantiene como constructor de tests por su uso
  extendido en la suite; el código de producción debe construir desde
  `worldmodel::World`.
- El fallback a `scenes.toml` y `LocalWorldService` sigue vigente en el cliente;
  un mundo sin portales o sin `farm`/`town` se rechaza al crear la partida.
