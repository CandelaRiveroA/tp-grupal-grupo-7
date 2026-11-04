# Contrato de convenciones del cliente

> **LEER ESTE ARCHIVO ANTES DE IMPLEMENTAR CUALQUIER COSA NUEVA.**
>
> Es el acuerdo de diseño del proyecto: las reglas que ya se decidieron y
> que NO hay que volver a discutir ni violar. Su objetivo es evitar que se
> implemente algo que ya fue vetado y tener que rehacerlo. Si una regla de
> acá entra en conflicto con lo que parece más rápido, gana la regla.
>
> Si aparece una convención nueva durante el trabajo, se agrega acá.

---

## 1. Estructura de código (headers vs. cpp)

- **Nada de lógica en los headers.** Los `.h` llevan SOLO: declaraciones de
  clases/structs, campos de datos, firmas de métodos, enums y comentarios.
  Todo método con cuerpo (cálculos, loops, `return` con lógica) va al `.cpp`.
- **Única excepción:** los **templates** deben vivir en el header (es una
  restricción de C++, no una elección). Ej: `TomlDocument::getIntArray<N>`.
- **Constantes `inline constexpr`** (datos, sin lógica) SÍ pueden estar en
  el header; no son lógica. Ej: `core/Constants.h`, `world/SceneKeys.h`.
  Esto NO aplica a funciones con cuerpo (esas van al cpp aunque sean de una
  línea, ej. `assetPath`/`soundPath` -> `Constants.cpp`).
- **Headers chicos y enfocados.** Si un header crece mucho (orientativo:
  >300 líneas) suele ser señal de que mezcla datos con lógica: separar.

## 2. Nada global, nada static, nada de funciones libres

- **Prohibido el estado global** y las variables `static` (de archivo o de
  función) que guarden estado.
- **Prohibidas las funciones libres** sueltas en el namespace para lógica.
- **Prohibidos los métodos `static`** como "helpers". Si algo es un helper
  de una clase, va como **método de instancia** (privado si es interno).
- Regla general: **siempre preferir lo que vive dentro de una clase.** Si
  una traducción/cálculo se comparte, se expone como método (o constructor)
  de la clase dueña del concepto, no como función libre ni static.
  - Ejemplo aplicado: la traducción capa-TOML -> dominio es el **constructor**
    `world::ConfigError(const tomlcfg::TomlError&)`, no una función libre.

## 3. Manejo de errores

- **Un único archivo de enums/tipos de error de configuración:**
  `client/world/ConfigError.h` / `.cpp`. Define `enum ConfigErrorKind` y la
  clase `ConfigError`. **Ningún loader define su propio tipo de error**: todos
  usan `world::ConfigError` por referencia.
- **Cero strings de mensaje armados a mano.** Nada de
  `throw X(kind, "campo " + "debe ser...")`. Prohibidas las frases de dominio
  concatenadas en los call sites.
- El error lleva SOLO: la **categoría** (`ConfigErrorKind`) y el **path crudo**
  de la clave TOML que falló (ej. `"rooms.bedroom.scale"`). El path es un DATO
  de ubicación (el que ya da `tomlcfg::TomlError::path()`), no un mensaje.
- El texto legible, si hace falta, lo compone **un solo lugar** (el borde que
  muestra, con el Logger) a partir de la categoría. Los loaders solo lanzan
  `kind + path`.
- **Un solo `catch`** en el borde del juego (`Application::runGame`) para
  `world::ConfigError`, no uno por cada subsistema.

## 4. Preferencias de estilo de implementación

- **Diccionario sobre switch.** Para mapear un valor a otro, preferir un
  `std::map`/tabla sobre un `switch`. (Nota: el `switch` sobre `enum class`
  da chequeo de exhaustividad con `-Wswitch`; el mapa no. Aun así, la
  preferencia del proyecto es el diccionario.)
- **enum class** para vocabulario (no números sueltos): `CreatureKind`,
  `ItemKind`, `ActionKind`, `Layer`, etc.

## 5. Datos en TOML, no en constantes C++

- Todo lo que es **presentación / apariencia** y NO le importa al servidor
  (tamaños de celda, posiciones, escalas, paddings, colores, celdas de
  sprite, layout) va en un **TOML**, no como constante compilada.
- Lo que SÍ es vocabulario del juego / se comunica con el servidor (acciones,
  tipos de ítem, direcciones) va como **enum en C++**. Los VALORES visuales
  de esos conceptos (qué celda los dibuja, tamaños) van al TOML.
- Patrón de carga establecido: `archivo.toml` -> `XxxTomlReader`/`Loader`
  (traduce y valida, lanza `ConfigError`) -> struct de **datos puros sin SDL**
  -> la capa SDL (`Renderer`/`InventoryHud`) lo dibuja.
- Las hojas de sprites se resuelven por `SheetCache` (una textura por imagen,
  cacheada); soporta varias hojas distintas a la vez.

## 6. Separación estado de juego vs. presentación

- **Qué hay / qué pasa** (inventario del jugador, entidades, acción actual) es
  **estado de juego**: viaja por `game::LocalWorldView`. Hoy lo llena
  `MockGameClient`; mañana el `GameClient` de red. El render NO decide esto,
  solo dibuja lo que la vista indica.
- **Cómo se ve / dónde se dibuja** es **presentación**: vive en los TOML y en
  la capa SDL. No mezclar con el estado de juego.
- Al conectar el servidor, el único cambio debe ser **quién llena la vista**
  (mock -> red); el dibujo no se toca. Dejar siempre ese "seam" listo.

## 7. Verificación (obligatoria antes de dar algo por hecho)

- Tras cualquier cambio: **compilar** (`cmake --build build`) y **correr la
  suite completa** (`./build/taller_tests`). No dar por terminado sin verde.
- **TDD** para lógica nueva (loaders, geometría): primero/también tests.
- Correr SIEMPRE la suite entera tras cambios transversales (no solo el
  subsistema tocado): un cambio de datos puede romper un test de integración
  de otro lado.
- **Excepción acordada: no sumar tests nuevos sobre infraestructura
  temporal** (mocks/bloques demo que van a desaparecer, ej.
  `MockGameClient`, los demos del "server falso"): son deuda que muere
  con el código. La verificación de eso es la suite EXISTENTE (sin
  regresiones) + la prueba visual. Si algo falla, recién ahí se escribe
  el test que lo reproduce. Cuando exista el servidor real, los tests se
  escriben contra las vistas reales.

## 8. Herramienta de inspección (provisoria)

- `taller_tile_inspector` es una herramienta aparte (opción CMake
  `TALLER_TILE_INSPECTOR`, OFF por defecto) para relevar celdas de hojas de
  sprites. Está aislada del cliente real; no pasa por lobby ni intro.
- **Importante:** si se reconfigura el build sin `-DTALLER_TILE_INSPECTOR=ON`,
  el target queda OFF y no se regenera (se puede quedar corriendo un binario
  viejo). Para refrescarlo: `cmake -S . -B build -DTALLER_TILE_INSPECTOR=ON`
  y recompilar.

## 9. Principios de diseño (obligatorios)

- **Cada cambio debe ser ESCALABLE.** Pensar cómo crece: agregar la próxima
  cosa parecida debería ser sumar datos / una entrada, no copiar código.
- **Distribuir responsabilidades CON SENTIDO para evitar acoplamiento.** Cada
  clase hace UNA cosa y la hace bien. El que DIBUJA (render) solo dibuja lo
  que recibe: no decide QUÉ dibujar (eso es de la capa de presentación/estado
  que tiene ese conocimiento). Si una clase empieza a tomar decisiones que no
  son suyas (ej. el render eligiendo la acción a mostrar), es acoplamiento:
  mover esa decisión a la clase dueña del concepto.
- **AVISAR cuando un archivo/clase empieza a tener más responsabilidades de
  las que le corresponden.** Si una clase crece hacia varias
  responsabilidades (orientativo: se vuelve grande —cientos de líneas— o su
  nombre deja de describir todo lo que hace), señalarlo y proponer separarla,
  NO seguir amontonando. Crear una clase para una responsabilidad no habilita
  llenarla de 400 líneas para un solo propósito: si crece así, está mal y hay
  que repensar la arquitectura.
- **Buscar lo GENÉRICO.** Si dos partes hacen lo mismo por motivos distintos,
  no duplicar archivos/lógica: generalizar en un único mecanismo
  parametrizado. Evitar "un archivo por caso" cuando el caso es el mismo
  patrón (ej.: el 9-slice de la barra y el del borde interno usan el mismo
  `nineSliceInside`; el error de config es uno solo para los 3 loaders).
  - Matiz: generalizar cuando el patrón YA se repite o es inminente, no
    especular con abstracciones sobre casos que todavía no existen.
- **Geometría separada del pintado.** El cálculo de dónde cae cada sprite
  vive en clases puras de `world/` (`ElementLayout`, `CharacterLayout`,
  `ToolOverlayLayout`, ...). La capa `sdl/` solo pinta (`ElementPainter`,
  `ToolOverlayPainter`, `EntityRenderer`). Así la geometría se testea sin SDL
  y el render no reimplementa aritmética.
- **Opciones de dibujo por tabla/mapa miembro, no por switch.** Cuando un
  enum decide cómo se dibuja (`DrawMode`, `FrameAnchor`, ...), la clase dueña
  del concepto lleva un mapa/tabla inicializado en su constructor y lo
  consulta por `.at(...)`. Nada de `switch` ni cadenas largas de `if` para
  opciones.
- **Constantes por tema, sin `static` ni `inline`.** Las constantes
  inmutables de una capa viven en archivos `*Constants.h` por dominio
  (`RenderConstants.h`, `WorldConstants.h`, `GameConstants.h`, ...) como
  `constexpr` de namespace. No usar `static` ni `inline` para constantes
  propias. Los datos grandes, como catálogos o arrays, se definen en `.cpp`.
- **Respetar los principios:**
  - **POLA** (menor asombro): la interfaz hace lo que su nombre sugiere;
    nada de efectos ocultos ni sorpresas.
  - **POLK / ocultamiento de conocimiento**: cada módulo esconde sus
    decisiones internas; los demás no dependen de cómo lo hace por dentro
    (ej.: solo `TomlDocument` conoce toml++; solo el mapper sabe celda->px).
  - **TDA** (tipos de dato abstractos): exponer operaciones, no la
    representación interna; el estado es privado y se accede por métodos.
  - **SOLID**: responsabilidad única por clase; abierto/cerrado (extender por
    datos o nuevas entradas, no modificando lo existente); sustitución
    (programar contra interfaces, ej. `GameClient` y no el mock); interfaces
    chicas y enfocadas; depender de abstracciones (datos/TOML + interfaces),
    no de detalles concretos.


## 10. Acciones del personaje y uso de herramienta

- El personaje se anima por **acciones** (`world::ActionKind`): idle, walk,
  use_tool, … Cada acción es una secuencia de poses por dirección, descrita
  en `characters.toml` (datos), con `loops` (cicla vs. one-shot). Agregar una
  acción = un valor en el enum + sus datos en el TOML, sin tocar el dibujo.
- La **acción actual es estado de juego** (`ViewEntity::action`): la decide
  el servidor; hoy el mock. El cliente solo ANIMA la acción que recibe (los
  cuadros, su duración, volver a idle son presentación). El servidor NO manda
  cuadros; manda "qué acción": el reparto no se reversiona al conectar la red.
- **Usar herramienta es genérico.** La tecla de "usar" (hoy Espacio) NO está
  acoplada a "picar": es una acción de input **rebindeable**. La acción
  concreta (picar, regar, …) dependerá de la **herramienta activa**
  (seleccionada del inventario, ej. tecla 2 = pico, 3 = regadera). El mapeo
  herramienta→acción debe ser dato/tabla, no casos hardcodeados. Hoy:
  herramienta activa fija = pico, un toque de "usar" = un golpe (one-shot).
- **Un toque = una acción.** "Usar" se dispara en el evento de tecla (keydown),
  una vez por pulsación (no sostenido): el golpe se reproduce entero y vuelve
  a idle. Quién prende/apaga la acción es la fuente de estado (mock/servidor),
  no el cliente; el cliente solo reproduce mientras la acción esté activa.
- **Cada acción tiene su propio ritmo (dato, no código).** Los ticks por
  cuadro salen de los datos de la acción (`frame` en el TOML, con default
  `cadence.frame` de la criatura): hacer más ágil el pico sin tocar la
  caminata es cambiar un dato. La duración de la ventana del golpe
  (`game/ToolUse`) se construye desde los MISMOS datos (`ActionData`) que usa
  el avance de cuadros, así ventana y animación no pueden desfasarse.
- **Efecto de herramienta genérico (dato, no código).** Una herramienta
  puede declarar, de forma OPCIONAL, un `effect`: otro overlay animado con su
  propia hoja, que larga mientras hace su acción (el chorro de la regadera al
  regar). Es el MISMO dato de animación que el overlay de la herramienta
  (`world::AnimatedOverlay`): hoja + celda + `frame_ticks` + `action` +
  `frames`. Se declara como subtabla `[tool.dir.effect]` en
  `tools_overlay.toml` y lo dibuja `sdl::ToolOverlayPainter` con el mismo
  `world::ToolOverlayLayout` que la herramienta (método `drawOverlayFrames`,
  compartido). Agregar un efecto a otra herramienta = solo datos, sin tocar
  el dibujo. El efecto es puramente visual (cae, impacta y desaparece); la
  consecuencia persistente en el terreno —si la hay— es otra cosa
  (`ViewGroundEffect`), que la decide la fuente de estado.
- **Capas con offset en celdas.** Una capa de un frame del overlay puede
  llevar un desplazamiento EN CELDAS respecto del ancla del frame:
  `[col, row, cols, rows, dx, dy]` (sin `dx, dy` = `0, 0`, el caso de una
  herramienta de una sola celda). Así un frame con VARIAS capas las
  distribuye en 2D (las gotas del chorro) en vez de apilarlas en el mismo
  punto. La geometría la resuelve `world::ToolOverlayLayout::layerRect`; el
  render no reimplementa la aritmética.
- **Tamaño normalizado a una celda (`cell_span`).** Un frame puede declarar
  `cell_span = [cols, rows]`: la caja del patrón en celdas. Si está, el frame
  entero se dibuja metido en UNA celda lógica (cada capa mide
  `celda / cell_span`), así un grupo de pocas gotas y uno lleno ocupan el
  MISMO tamaño en pantalla (el chorro crece en densidad, no en tamaño, y
  queda del tamaño de la celda de tierra). Sin la clave, cada capa mide una
  celda entera (el caso de la herramienta). Vive en `OverlayFrame`; lo aplica
  `ToolOverlayLayout::layerRect`.

## 11. Cámara del mundo

- La cámara es **presentación**, no estado de juego. Vive en
  `sdl::Camera` (`client/sdl/render/Camera.h`) y no conoce entidades,
  animaciones, red ni SDL: solo recibe rectángulos en píxeles de mundo y los
  traduce a píxeles de pantalla.
- Sigue el **rectángulo dibujado del jugador local**, no su celda lógica. El
  rectángulo sale de `ViewMotion::drawBoxFor(...)` (celda actual + offset de
  interpolación), así la cámara también queda centrada durante el paso.
- Se **clava en los bordes del mundo**: el offset se limita al rango
  `[origen, origen + tamañoMundo - tamañoViewport]`. Si el mundo es más chico
  que la ventana en un eje, ese eje queda fijo en el origen.
- El tamaño del mundo se obtiene con
  `ScenePixelMapper::worldBounds(scene.grid)`. El mapper sigue siendo el
  único dueño de la conversión grilla → píxeles.
- `Camera` no se pasa al renderer: produce un `sdl::Viewport`
  (`client/sdl/render/Viewport.h`) con offset, tamaño de pantalla,
  `toScreen(...)` y `intersects(...)`. El `Renderer` consume **solo** el
  `Viewport`, así se puede reemplazar la cámara sin acoplar el dibujo.
- El `Viewport` se aplica **solo a la capa de mundo** (elementos, recursos,
  items caídos y entidades). HUD, inventario, banners e intro/fade no se
  desplazan.
- Todo dibujo nuevo del mundo debe pasar por
  `viewport.toScreen(...)` antes de llegar a SDL. No se duplica la fórmula de
  cámara en el renderer.
- **Culling por contrato:** si el rectángulo final de pantalla no intersecta
  el viewport, ese draw no se emite. El culling no puede cambiar el resultado
  visual; por eso los elementos usan sus bounds reales (`ElementLayout`) y
  las entidades/resources/items tienen margen para overhangs de sprite.
- `TileLayout` ya no devuelve `std::vector` por frame: expone
  `visibleTiled(...)` / `visibleStrip(...)` como rangos de índices y
  `tiledAt(...)` / `stripAt(...)` para consultar cada pieza.
- `world::ChunkGrid` es el índice espacial común. Su tamaño de chunk se toma
  de la celda lógica (`ScenePixelMapper::cellPixelSize`), sus claves se
  empaquetan en un `std::uint32_t` y guarda índices `std::uint32_t`.
  `SceneElementIndex` indexa los elementos estáticos (se reconstruye con
  `Renderer::prepareScene`) y `game::ViewSpatialIndex` indexa el snapshot
  dinámico una vez por frame. Ninguno de los dos conoce assets ni SDL.
- La fórmula (centro, clamp y traslación) y sus constantes viven en
  `Camera.cpp`, centralizadas en un único cálculo de eje reutilizado para X e
  Y. Si en el futuro se agrega zoom, deadzone o márgenes, esa lógica se separa
  a un archivo propio sin tocar el renderer.
- `sdl::EntityRenderer` es el único que sabe dibujar una `game::ViewEntity`.
  Resuelve la apariencia por `kind` con `CharacterCatalog`, dibuja el cuerpo
  con `CharacterSprite` y delega la herramienta en `sdl::ToolOverlayPainter`.
  `Renderer` solo lo invoca; no conoce `Pose`, `ActionKind` ni overlays.
- **Fuera de alcance de esta rama:** assets, `AssetStore`, `SpriteSheet`,
  `SheetCache`, precarga, atlas/batching, animales, ids, protocolo, sesiones y
  concurrencia. El próximo refactor de assets debe entrar sin cambiar cámara,
  viewport, culling ni índices.

## 12. Contrato con la fuente de estado (campo, inventario y herramientas)

Lo que SDL ya provee para cuando la fuente de estado (el servidor) lo
mande. La fuente solo llena `LocalWorldView`; SDL decide cómo se ve y suena.

- **Ids de contenido = los de `crops.toml`.** Los sacos (`sack_id`, ej.
  `strawberry_seeds`) y las cosechas (la clave del cultivo, ej.
  `strawberry`) viajan por `content_id` en el inventario y en los ítems del
  piso. `world::FarmCatalogs::withCropContents` los registra en el catálogo
  del inventario al arrancar la sesión, así el HUD, el picker y el carry
  los dibujan sin tocar la red. Un id nuevo = una entrada en `crops.toml`.
- **La herramienta usada viaja en `ViewEntity::activeTool`.** La fuente
  llena la herramienta REAL (pico, pala, regadera), no una fija. SDL deriva
  de ahí la animación (`game::ActionResolver`, tabla herramienta ->
  acción: regadera -> `Water`; sin fila = `UseTool`) y el sonido
  (`core::ToolUseSounds`, tabla herramienta -> sonido). Si la fuente manda
  directamente una acción concreta (`Water`, `Milk`, ...), se respeta tal
  cual.
- **El slot es selección LOCAL.** Seleccionar un casillero del inventario
  (teclas 1..N, N = `InventoryLayout::count`) no viaja al servidor. Qué
  herramienta se usa y si se puede lo decide la fuente.
- **Presentación local del campo** (`game::FarmSelection`): lo que se
  sostiene sobre la cabeza (contenido `droppable` del slot activo) y el
  marcador de plantado bajo el puntero salen de la selección del HUD; el
  estado (`plantedCrops`, `groundEffects`) lo manda la fuente.
- **Sumar un sistema del campo** = una fila en la tabla que corresponda
  (`toolActions_`, `ToolUseSounds`) + sus datos en TOML; `PlaySession` solo
  compone y llama, no decide.
