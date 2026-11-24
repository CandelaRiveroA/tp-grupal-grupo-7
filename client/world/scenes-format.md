# Formato de escenarios (`scenes.toml`)

Los escenarios se describen como datos en `client/sdl/assets/scenes.toml`.
El código de dibujo es **genérico**: lee este archivo y dibuja lo que
indique, sin conocer qué es una pared, un piso u otro elemento. Para
agregar o cambiar un elemento se edita este archivo; **no hace falta
recompilar** el cliente.

> **La grilla de este archivo es de PRESENTACIÓN** (dónde se *dibuja* cada
> cosa). No describe colisiones ni reglas de juego: eso es autoridad del
> servidor. Un tile de pared acá es "dibujá esto en esta celda", no "acá
> no se puede pasar".

El archivo lo carga `world::SceneConfigLoader` hacia un
`world::SceneDescription` (datos puros, sin SDL), y el `sdl::Renderer` lo
dibuja.

---

## 1. Tilesets

Un tileset es un recurso gráfico: una imagen y el tamaño de celda de SU
grilla (en píxeles). Se define como una tabla `[tilesets.<nombre>]`.

```toml
[tilesets.farm]
image = "Farm/walls&floors.png"   # ruta relativa al directorio de assets
cell  = [16, 16]                  # [ancho, alto] de celda, en píxeles
```

| Campo   | Tipo         | Obligatorio | Descripción                           |
|---------|--------------|-------------|---------------------------------------|
| `image` | string       | sí          | Ruta de la imagen (relativa a assets) |
| `cell`  | `[int, int]` | sí          | Tamaño de celda de la grilla (px)     |

Agregar un tileset nuevo = otra tabla `[tilesets.<otro>]`.

---

## 2. Habitaciones

Una habitación se define como `[rooms.<nombre>]` y tiene color de fondo,
el tamaño de su grilla lógica, una escala de dibujo por defecto y una
lista de elementos.

```toml
[rooms.bedroom]
floor_color = [0, 0, 0, 255]   # RGBA del fondo (lo que no cubre un tile)
grid        = [30, 20]         # tamaño de la habitación en celdas lógicas
scale       = 2                # escala de dibujo por defecto (entera)
sounds      = ["chicken_cluck"] # cues audibles mientras se renderiza (opcional)
```

| Campo         | Tipo            | Obligatorio | Descripción                              |
|---------------|-----------------|-------------|------------------------------------------|
| `floor_color` | `[int,int,int,int]` | sí      | Color de fondo RGBA (0..255 por canal)   |
| `grid`        | `[int, int]`    | sí          | Celdas lógicas `[columnas, filas]`       |
| `scale`       | int             | sí          | Escala por defecto de los elementos      |
| `sounds`      | `[string, ...]` | no          | Cues de sonido audibles en la sala       |

**`sounds` (opcional; sin la key la sala es muda):** lista de cues de
sonido que la presentación reproduce MIENTRAS esta sala se renderiza.
Quien llena la vista (el mock hoy, el server mañana) emite eventos
per-tick (`LocalWorldView.soundEvents`, cada uno con cue + id del
emisor); un evento solo suena si la sala actual declara su cue — ej.
las gallinas cacarean solo donde están. Cuando el granero sea una sala
propia, su lista declarará los sonidos de animales y la de este cuarto
dejará de hacerlo: **cambio de TOML, cero código**.

Una **celda lógica** mide, en píxeles, `cell_del_tileset × scale`. Con
`cell = [16,16]` y `scale = 2`, cada celda lógica es de 32×32 px; una
grilla `[30, 20]` cubre entonces 960×640 px.

---

## 3. Elementos

Cada elemento es un item de la lista `[[rooms.<room>.elements]]`. Agregar
un elemento = agregar un bloque como este. **El orden en el archivo es el
orden de dibujo**: lo que está más abajo se pinta encima.

```toml
  [[rooms.bedroom.elements]]
  tileset = "farm"            # qué tileset usar (definido en [tilesets])
  region  = [0, 27, 2, 2]     # qué celdas de la imagen: [col, row, cols, rows]
  draw    = { mode = "tile" } # cómo dibujar (ver abajo); hereda scale
  at      = [0, 2]            # celda lógica inicial [columna, fila]
  span    = [30, 18]          # área lógica que cubre [columnas, filas]
```

| Campo     | Tipo                 | Obligatorio | Descripción                                          |
|-----------|----------------------|-------------|------------------------------------------------------|
| `tileset` | string               | sí          | Nombre de un tileset definido en `[tilesets]`        |
| `region`  | `[int,int,int,int]`  | sí          | Región en la imagen: `[col, row, cols, rows]` (celdas) |
| `draw`    | tabla inline         | sí          | Modo de dibujo y escala (ver abajo)                  |
| `at`      | `[int, int]`         | sí          | Celda lógica donde arranca `[columna, fila]`         |
| `span`    | `[int, int]`         | sí          | Área lógica que cubre `[columnas, filas]`            |
| `enter`   | tabla inline         | no          | Hace "entrable" al elemento (ver abajo)              |

- `region` describe un rectángulo dentro de la imagen, en celdas: empieza
  en `(col, row)` y abarca `cols × rows` celdas.
- `at`/`span` ubican el elemento en la **grilla lógica** de la habitación.

### `draw`

```toml
draw = { mode = "horizontal_strip", scale_x = 2, scale_y = 3 }
```

| Campo     | Tipo   | Obligatorio | Descripción                                        |
|-----------|--------|-------------|----------------------------------------------------|
| `mode`    | string | sí          | `single`, `tile` o `horizontal_strip`              |
| `scale`   | int    | no          | Escala uniforme; si falta, hereda la de la room    |
| `scale_x` | int    | no          | Escala horizontal (sobrescribe `scale`/default)    |
| `scale_y` | int    | no          | Escala vertical (sobrescribe `scale`/default)      |

**Herencia de escala:** si un elemento no declara nada, usa el `scale` de
la habitación. Si declara `scale`, lo usa en ambos ejes. `scale_x`/
`scale_y` mandan por eje si están presentes.

**Modos de dibujo:**

- `single`: dibuja la región **una sola vez**, anclada en `at`.
- `tile`: repite la región en **mosaico 2D** hasta cubrir el `span`
  (recorta la última fila/columna si no entra justa).
- `horizontal_strip`: repite la región **solo a lo ancho** del `span`;
  el alto es el de la pieza dibujada (su `scale_y`). Recorta la última
  pieza en ancho para cubrir exacto.

### `enter` (opcional)

```toml
enter = { rows = [2, 3], lie = [20, 13], facing = "back" }
```

Hace **entrable** al elemento: el jugador puede meterse con el click
del cursor (presentación local del cliente; la fuente de estado no
participa). Al meterse, el jugador se ubica en `lie` mirando `facing` y
las filas `rows` de la pieza se dibujan **encima de las entidades** (lo
"tapan"). En la cama: la cabecera queda detrás —la cabeza se ve sobre
la almohada— y las frazadas (mitad + pie) quedan delante del cuerpo.
Sale con otro click sobre el elemento, con un click afuera o al moverse.

| Campo    | Tipo         | Obligatorio | Descripción                                            |
|----------|--------------|-------------|--------------------------------------------------------|
| `rows`   | `[int, int]` | sí          | Filas de la pieza (inclusive, desde 0) que van adelante |
| `lie`    | `[int, int]` | sí          | Celda lógica donde queda el jugador `[col, fila]` (dentro de `at`/`span`) |
| `facing` | string       | sí          | Pose del jugador: `front`, `back`, `left`, `right`      |

Otra cama (o cualquier mueble donde meterse) = otra entrada de elemento
con su `enter`: sin código nuevo.

---

## 4. Ejemplo completo: el dormitorio

```toml
[tilesets.farm]
image = "Farm/walls&floors.png"
cell  = [16, 16]

[rooms.bedroom]
floor_color = [0, 0, 0, 255]
grid        = [30, 20]
scale       = 2
sounds      = ["chicken_cluck"]

  # Piso: bloque lógico de 2x2 celdas desde (col 0, fila 27), en mosaico.
  [[rooms.bedroom.elements]]
  tileset = "farm"
  region  = [0, 27, 2, 2]
  draw    = { mode = "tile" }
  at      = [0, 2]
  span    = [30, 18]

  # Pared superior: columna de 3 celdas repetida a lo ancho.
  [[rooms.bedroom.elements]]
  tileset = "farm"
  region  = [0, 0, 1, 3]
  draw    = { mode = "horizontal_strip", scale_x = 2, scale_y = 3 }
  at      = [0, 0]
  span    = [30, 2]
```

---

## 5. Cómo agregar una pared (o cualquier elemento) nuevo

1. Asegurate de que el tileset que la contiene esté definido en
   `[tilesets.<nombre>]`. Si usás una imagen nueva, agregá su tabla.
2. Averiguá la **región** del elemento en la imagen, en celdas:
   `[col, row, cols, rows]`.
3. Decidí **dónde** va en la grilla lógica (`at`) y **cuánto** ocupa
   (`span`), y con qué **modo** se dibuja.
4. Agregá un bloque de elemento al final de la lista de la habitación:

   ```toml
     [[rooms.bedroom.elements]]
     tileset = "farm"
     region  = [3, 0, 1, 3]     # las celdas de la pared nueva
     draw    = { mode = "horizontal_strip", scale_x = 2, scale_y = 3 }
     at      = [0, 0]           # dónde empieza en la grilla lógica
     span    = [30, 2]          # qué ancho/alto lógico cubre
   ```

5. Guardá y volvé a correr el cliente. **No hace falta recompilar código
   C++.** El escenario se arma desde este archivo.

Recordá: el orden en el archivo es el orden de pintado. Si la pared debe
quedar por encima del piso, ponela **después** en la lista.

---

## 6. Mapas del editor y `overlay.toml`

Además de las salas de este archivo, el cliente puede dibujar los **mapas
que guarda el editor** (`world.toml`, formato de `common/world_model/`).
Si `world.toml` no existe, el cliente funciona solo con `scenes.toml`,
como siempre. Si existe, sus mapas (`farm`, `town`, `market`, `cabin`,
...) ganan sobre las salas de `scenes.toml` con el mismo id.

El editor decide **dónde** está cada cosa. `overlay.toml` (opcional, mismo
directorio de assets) dice lo que el editor no puede decir:

| Sección | Qué hace |
|---|---|
| `[world] scale` | px de pantalla por px de arte (un solo valor para todo el mapa; default 2). La celda lógica es `tile_size * scale`. |
| `[tilesets.<n>]` | hojas propias del cliente (misma sintaxis que arriba) para decoraciones iniciales y variantes. No pueden redefinir una hoja del editor con otra imagen. |
| `[decorations."<tileset:col,row>"] night_tile` | la decoración que el editor puso con ese tile de origen se dibuja de día con su tile y de noche con `night_tile` (misma pieza y tamaño). |
| `[decorations."..."] enter = { rows, facing, marker }` | la decoración es "entrable" (la cama): `rows` son las filas que tapan al jugador y la celda donde queda acostado sale del marker `marker` (default `bed`) del mapa, no se copia a mano. No se combina con `night_tile`. |
| `[markers.<kind>] npc` | el NPC de `npcs.toml` que se ubica en ese marker (`merchant_pierre` -> `pierre`). Es su celda por defecto: la que mande el server la pisa. |
| `[roles.<role>] action, target` | qué hace el click sobre un edificio de ese rol: `popup` (abre el popup del NPC `target` de `npcs.toml`) o `enter_map` (pide pasar al mapa `target`). |
| `[[maps.<id>.elements]]` | decoraciones **iniciales** de un mapa (lo que el cliente pone en el primer arranque). Misma sintaxis que los elementos de una sala; solo pueden usar hojas de `[tilesets.*]` de este archivo. |

Reglas que se respetan al armar la escena (si no, `ConfigError` con la
categoría y el path de la clave que falló):

- **Coherencia en celdas.** Una celda del editor es una celda del server y
  de la escena. Un objeto de `size = [w, h]` ocupa exactamente `w x h`
  celdas. La celda de arte de cada hoja debe dividir exacto a la celda
  lógica (si no, no se deforma: es un error).
- **Orden.** Piso (`ground`), caminos (`paths`), objetos del editor en el
  orden del archivo y, al final, las decoraciones iniciales del overlay.
- **Profundidad.** Los objetos del editor se ordenan con las entidades por
  la fila de su base: el jugador pasa por detrás de un árbol y por delante
  si está más abajo. Las decoraciones iniciales del overlay van debajo de
  todo, como cualquier elemento de este archivo.
- **Avisos.** Una entrada del overlay que no coincide con nada (un tile mal
  escrito, un mapa o rol que no existen) no frena el juego: se avisa por el
  log con su path.
