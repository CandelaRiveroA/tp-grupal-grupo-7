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
