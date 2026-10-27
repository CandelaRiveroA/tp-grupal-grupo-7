# Punto de entrada: animales

Documento para quien conecta **cliente ↔ servidor**. Explica **dónde** entra el
movimiento automático de los animales (gallinas, vacas, …) al cliente, y qué
parte es **lógica de juego** (del lado del servidor) y qué parte es
**presentación** (del cliente). Es el equivalente, para los animales, de lo que
`connection-point.md` describe para `GameClient` y `inventory-entry-point.md`
para el inventario.

El cliente es **dummy**: el servidor decide **dónde** está cada animal, hacia
dónde mira y si camina; el cliente solo **interpola y dibuja**. Hoy esa
"autoridad del servidor" la simula el mock; al conectar la red, la misma lógica
pasa al servidor y el cliente no cambia.

## TL;DR
- El servidor (hoy: `MockGameClient`) decide **celda, dirección y si camina**
  de cada animal, una vez por tick.
- La decisión vive en `game::MockAnimalSimulation`
  (`client/game/MockAnimalSimulation.{h,cpp}`): lógica pura, sin SDL ni vista.
- Cada tick el loop llama `GameClient::update(it)` (UNA llamada, al inicio del
  tick) → la simulación avanza → `worldState()` devuelve la vista actualizada.
- Las gallinas también **ponen huevos**: aparecen como ítems en el suelo
  (`LocalWorldView.groundItems`) que el render dibuja bajo las criaturas.
- Y **cacarean**: eventos de sonido per-tick (`LocalWorldView.soundEvents`)
  que la presentación reproduce si la sala renderizada declara el cue.
- El render ya dibuja cualquier criatura: la hoja se resuelve por
  `entity.kind` vía el catálogo (`characters.toml`); **agregar una criatura no
  toca el render**.

## El flujo, paso a paso
```
servidor  (hoy: MockGameClient simula)  decide el movimiento de los animales
   └─► game::MockAnimalSimulation::update(it)      lógica pura (wander determinista)
         └─► MockGameClient::rebuildView()     vuelca cada animal a un ViewEntity
               └─► GameClient::worldState()    RECIBIR (ver connection-point.md)
                     └─► ViewMotion::observe(view, it)   interpola EL PASO del animal
                           └─► Renderer::drawEntity       ◄── PRESENTACIÓN (solo dibuja)
                                  ├─ hoja del animal  ← catálogo por entity.kind
                                  └─ offset de píxeles ← ViewMotion (por entity.id)
```

## Qué decide el "servidor" — contenido
`MockAnimalSimulation` es **lógica de juego**: no conoce SDL ni la vista, solo el
mundo en celdas. Por cada animal lleva:

```cpp
struct AnimalRecord {                    // DATOS puros (client/game/MockAnimalSimulation.h)
    int id;                              // estable entre frames (jugador = 0, animales 1..N)
    world::CreatureKind kind;
    int cellX, cellY;                    // celda ACTUAL (destino del paso si moving)
    protocol::Direction facing;
    bool moving;
    AnimalActivity activity;             // Resting / Stepping
    int stepIteration, restTicks;
    world::MoveCadence cadence;          // dato del kind (del TOML de personajes)
};
```

Comportamiento (wander):
- **Resting**: espera `restTicks` aleatorio; al terminar elige dirección al
  azar y, si el destino es pisable, arranca **Stepping** (fija la celda
  destino y `facing` YA: la vista interpola el desplazamiento).
- **Stepping**: avanza el paso durante `cadence.stepTicks` ticks; al cerrar
  vuelve a **Resting**.
- **Colisiones**: no entra en celdas fuera de grilla, la del jugador ni de
  otro animal. Es una **regla de juego silenciosa**, no un error: si el
  destino está ocupado, reintenta al tick siguiente.
- **Determinismo**: el generador es `std::mt19937(seed)`; con la misma seed el
  wander es idéntico (testeable).

Los **datos de spawn** (celdas y seed) hoy viven como tabla `constexpr` en
`MockGameClient.cpp` (dato del server falso, precedente: `kCreatureBindings`).
Al conectar la red, quien spawnea pasa a ser el servidor y esto se reemplaza.

## Qué ve el cliente — vista y animación
- Cada animal llega a la vista como `ViewEntity` (`LocalWorldView.h`) con su
  **`id` estable**, igual que el jugador. El render lo dibuja igual que a
  cualquier criatura: hoja + celdas del catálogo por `entity.kind`.
- `ViewMotion` empareja track ↔ entidad **por `id`** (no por posición en la
  lista): la interpolación de cada animal sobrevive al reordenamiento de la
  vista entre frames. Una id nueva se dibuja plantada en su celda.
- El sonido del paso (`FootstepCue`) aplica igual que para el jugador: sale
  del mismo cálculo de fase (`isWalking` + cuadros de walk del catálogo).

## Los huevos: postura → suelo (recoger queda para después)
Las gallinas **ponen huevos**; es la misma lógica de "server falso":

1. **Postura** (`MockAnimalSimulation`): cada animal pone un huevo cada
   `layPeriod` ticks (al azar en una gama LARGA al spawn, y reasignada tras
   cada postura: las gallinas no se sincronizan). El huevo queda en la celda
   **donde el animal está** al ponerlo. Qué ítem pone cada kind es una TABLA
   (`kLays`: chicken → `ItemKind::Egg`); mañana la vaca entra sumando una
   fila (leche), sin tocar código. Determinista con la seed (testeable).
2. **Drenaje** (`takeLaidEggs()`): el mock toma lo puesto en el tick y lo
   valida con sus reglas de juego silenciosas:
   - **Tope de ítems en el suelo** (`kMaxGroundItems = 3`): sin recogida, es
     lo que evita que la vista se llene de huevos. El server real definirá
     su propio tope/despawn.
   - **Celda ya ocupada por otro huevo**: la postura se descarta (no hay
     dos huevos en una celda).
3. **Vista**: cada ítem del suelo es un `ViewGroundItem` (id estable, celda,
   `ItemKind`) en `LocalWorldView.groundItems`. **No bloquea a nadie**: el
   jugador y las gallinas caminan encima.
4. **Dibujo**: el `Renderer` los dibuja entre los elementos del escenario y
   las criaturas (quedan DEBAJO de quien pise la celda), centrados en su
   celda, a tamaño `drawn` del catálogo visual de ítems (`inventory.toml`,
   `[items.egg]` — misma fuente de datos que los íconos del inventario, ver
   `world/inventory-format.md` §6).

**Fuera de alcance por ahora:** recoger el huevo y guardarlo en el
inventario. La vista ya lleva `ItemKind`, así que la recogida futura solo
agrega la regla del "servidor" (quitar del suelo + sumar slot) — el dibujo
del ícono ya existe.

## Los cacareos: sonido ambiental → evento per-tick
Las gallinas también **suenan**; mismo modelo que todo lo demás: el
"server" decide CUÁNDO, la presentación decide CÓMO suena.

1. **Decisión** (`MockAnimalSimulation`, patrón `tryLay`): cada gallina
   suelta un cacareo cada `cluckPeriod` ticks, al azar en una gama LARGA
   (`kMinCluckPeriodTicks`/`kMaxCluckPeriodTicks` = 1200–2700 ticks ≈ 20–45 s
   a 60 FPS) al spawn, y reasignada tras cada cacareo: las gallinas no se
   sincronizan entre sí y la vista no se inunda de sonidos. Qué cue suelta
   cada kind lo decide el **`game::SoundCueCatalog`** (atributo `cues_` de
   la simulación: `cueOf(kind)` → `CueKind` o nullopt). El chequeo usa un
   **stream de RNG separado** (`cluckRandom_`, seed derivada): tocar el RNG
   del wander/postura cambiaría esas secuencias deterministas que los tests
   pinean.
2. **Drenaje** (`takeClucks()` → `MockGameClient::update`): cada evento es
   un `ViewSoundEvent` (cue + `emitterId`, el id estable de la vista) y va
   a `LocalWorldView.soundEvents`. Son **per-tick**: cada `update()` los
   reemplaza (vacío si no hubo), no se acumulan ni son estado.
3. **Reproducción** (`Application.cpp`, junto al footstep): por cada
   evento del tick, el `game::SoundGate` (puro y testeado) consulta si
   la sala renderizada declara el cue (`sounds` en `scenes.toml`, ver
   `world/scenes-format.md` §2) y, si lo declara, `audio.playCue(cue)`:
   el `sdl::AudioPlayer` es dueño del diccionario cue → `SoundId`
   (atributo `cueSounds_`) y decide CÓMO suena cada cue localmente.
   **Agrega un sonido nuevo = una fila en `kSounds` (`AudioCatalog.h`)
   y una fila en `cueSounds_`; el gate por sala es puro TOML.**
4. **Con la red:** el server decide cuándo suena algo y a quién le interesa
   (hoy el mock emite siempre y el cliente silencia lo que su sala no
   declara; el server real puede filtrar ANTES de mandar, y así viaja
   menos). El mensaje "soná X (emisor Y)" llena el mismo `soundEvents` y
   el cliente no cambia.

**Mañana la vaca muuge** = valor en `CueKind` + fila en
`SoundCueCatalog` (nombres y emisores) + fila en `sdl::kSounds` +
fila en `cueSounds_` de `AudioPlayer` + su sala declarándolo en
`scenes.toml`. Cero lógica nueva.

## Integrar la implementación de red
1. Implementar una clase que herede de `game::GameClient` (`sendCommand` +
   `worldState` + `update`), como describe `connection-point.md`.
2. En `update(it)`, drenar el Receiver y actualizar el estado del mundo;
   en `worldState()`, llenar `entities` con un `ViewEntity` por animal que
   mande el servidor (jugador primero con `id = 0`, luego los animales con su
   id del servidor).
3. Nada más: `ViewMotion` interpola cada paso por id y `Renderer` ya dibuja
   la criatura por kind. **El cliente SDL no cambia.**
4. Referencia de comportamiento: `MockAnimalSimulation` es el modelo que espera la
   cátedra para la IA del server (wander simple en celdas, determinista si el
   server fija la seed). Portarla al servidor es copiar la clase: no depende
   de nada del cliente salvo `world::` (datos puros).

## Reglas / edge cases
- `ViewEntity.id` repetido entre entidades → el segundo pierde su track (el
  emparejamiento marca cada id como usada una vez por frame). En el estado de
  red real los ids son únicos.
- Un animal que **no** aparece en un frame deja de trackearse (si reaparece,
  nace sin historial: se dibuja plantado, sin deslizarse).
- `cadence.stepTicks` del animal lo aporta el catálogo (`characters.toml`);
  `MockAnimalSimulation` lo usa como ventana del paso, igual que la vista.
- Animal **fuera de la grilla o en la celda del jugador** al spawnear: el
  spawn es rechazado (falso, sin excepción).
