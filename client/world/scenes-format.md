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
```

| Campo         | Tipo            | Obligatorio | Descripción                              |
|---------------|-----------------|-------------|------------------------------------------|
| `floor_color` | `[int,int,int,int]` | sí      | Color de fondo RGBA (0..255 por canal)   |
| `grid`        | `[int, int]`    | sí          | Celdas lógicas `[columnas, filas]`       |
| `scale`       | int             | sí          | Escala por defecto de los elementos      |

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
