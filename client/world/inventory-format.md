# Formato de la barra de inventario (`inventory.toml`)

La barra de inventario se describe como datos en
`client/sdl/assets/inventory.toml`. El código de dibujo es **genérico**:
lee este archivo y dibuja lo que indique, sin hardcodear la hoja, el
cuadro ni la posición. Para cambiar la apariencia de la barra se edita
este archivo; **no hace falta recompilar** el cliente.

El enunciado fija el inventario en **9 objetos distintos**; por eso la
barra tiene 9 slots. Esta primera versión dibuja solo los cuadros
**vacíos** (sin ítems adentro).

El archivo lo carga `world::InventoryConfigLoader` hacia un
`world::InventoryLayout` (datos puros, sin SDL), y `sdl::InventoryHud` lo
dibuja.

---

## 1. Hoja (`[sheet]`)

La hoja de sprites y el tamaño de celda de SU grilla (en píxeles).

```toml
[sheet]
image = "Players/main/tools/MenuTiles.png"  # ruta relativa a assets
cell  = [16, 16]                             # [ancho, alto] de celda (px)
```

| Campo   | Tipo         | Obligatorio | Descripción                           |
|---------|--------------|-------------|---------------------------------------|
| `image` | string       | sí          | Ruta de la imagen (relativa a assets) |
| `cell`  | `[int, int]` | sí          | Tamaño de celda de la grilla (px)     |

---

## 2. Cuadro del slot (`[slot]`)

La celda `[columna, fila]` de la hoja que contiene el marco de un slot
vacío. Todos los slots de la barra usan el mismo cuadro.

```toml
[slot]
cell = [4, 8]   # col 4, fila 8 -> x=64, y=128 en píxeles de la hoja
```

| Campo  | Tipo         | Obligatorio | Descripción                              |
|--------|--------------|-------------|------------------------------------------|
| `cell` | `[int, int]` | sí          | Celda `[columna, fila]` del cuadro vacío |

La celda se descubrió con la herramienta de inspección
(`client/sdl/dev/tile_inspector_main.cpp`): con `cell = [16,16]`, la
columna 4 / fila 8 corresponde al rectángulo `x=64, y=128, w=16, h=16`.

---

## 3. Disposición de la barra (`[bar]`)

Cuántos slots hay, a qué escala se dibujan, cuánto se separan y dónde se
ancla la barra en la ventana.

```toml
[bar]
count  = 9              # cantidad de slots (límite del inventario)
scale  = 3              # escala entera (x3 -> slot de 16*3 = 48 px)
gap    = 0              # separación en px entre slots (0 = pegados)
anchor = "bottom_center" # ancla en la ventana
margin = 16             # separación en px desde el borde anclado
```

| Campo    | Tipo   | Obligatorio | Descripción                                        |
|----------|--------|-------------|----------------------------------------------------|
| `count`  | int    | sí          | Cantidad de slots (debe ser > 0)                   |
| `scale`  | int    | sí          | Escala de dibujo entera (debe ser > 0)             |
| `gap`    | int    | sí          | Separación en píxeles entre slots (>= 0)           |
| `anchor` | string | sí          | Posición en la ventana (ver abajo)                 |
| `margin` | int    | sí          | Separación en píxeles desde el borde anclado (>=0) |
| `ui_scale` | float | no         | Factor de tamaño de toda la barra (opcional, >0, default 1.0) |

**`ui_scale`** escala TODA la barra (cuadros + borde + solapamiento) como
una unidad, manteniendo las proporciones. `1.0` = tamaño base (lo que
definen `scale`/`thickness`/`inset`); `0.8` achica el conjunto al 80%;
valores > 1.0 lo agrandan. Acepta decimal (`0.8`) o entero (`1`).

**Valores de `anchor`:**

- `bottom_center`: barra abajo, centrada horizontalmente, separada
  `margin` px del borde inferior.

(Hoy solo se soporta `bottom_center`; agregar otro anclaje es extender el
reader y esta tabla.)

**Geometría resultante:** cada slot mide `cell × scale` px. El ancho de la
fila de slots es `count × (cell_w × scale) + (count − 1) × gap`. Si hay
marco (ver abajo), el conjunto total suma el grosor del borde por cada
lado y se centra con el marco incluido.

---

## 4. Marco decorado 9-slice (`[frame]`, opcional)

Envuelve la fila de slots con un borde de esquinas redondeadas. Es
**opcional**: si no está la sección `[frame]`, la barra se dibuja sin
marco. Técnica **9-slice**: las 4 esquinas se dibujan sin deformar (un
bloque de `corner_size × corner_size` celdas) y los 4 bordes repiten/
estiran su celda para cubrir cada lado. Bordes **lisos** (sin adorno
central).

```toml
[frame]
border_cells = 2       # espesor del marco en celdas (esquinas y bordes)
thickness    = 16      # grosor del borde en px (igual en los 4 lados)
top_left     = [1, 1]  # celda origen de cada pieza [columna, fila]
top_right    = [13, 1]
bottom_left  = [1, 13]
bottom_right = [13, 13]
top          = [3, 1]  # bordes: se estiran a lo largo del lado
bottom       = [3, 13]
left         = [1, 3]
right        = [13, 3]
```

| Campo          | Tipo         | Descripción                                        |
|----------------|--------------|----------------------------------------------------|
| `border_cells` | int          | Espesor del marco en celdas, esquinas y bordes (>0)|
| `thickness`    | int          | Grosor del borde en px, igual en los 4 lados (> 0) |
| `inset`        | int          | Px que el marco solapa hacia adentro (opcional, ≥0)|
| `top_left`     | `[int, int]` | Celda origen de la esquina superior izquierda      |
| `top_right`    | `[int, int]` | Celda origen de la esquina superior derecha        |
| `bottom_left`  | `[int, int]` | Celda origen de la esquina inferior izquierda      |
| `bottom_right` | `[int, int]` | Celda origen de la esquina inferior derecha        |
| `top`          | `[int, int]` | Celda origen del borde superior                    |
| `bottom`       | `[int, int]` | Celda origen del borde inferior                    |
| `left`         | `[int, int]` | Celda origen del borde izquierdo                   |
| `right`        | `[int, int]` | Celda origen del borde derecho                     |

**Espesor parejo:** todas las piezas recortan `border_cells` celdas de
espesor de la hoja, así esquinas y bordes tienen el mismo grosor de línea.
Las esquinas recortan un bloque `border_cells × border_cells`; el borde
superior/inferior recorta `1 × border_cells` (se estira a lo ancho) y el
izquierdo/derecho `border_cells × 1` (se estira a lo alto).

**Grosor del borde** = `thickness` px, igual para los 4 lados e
independiente de la escala del slot. El marco va **por fuera** de los
slots: el conjunto total mide `fila_de_slots + 2 × thickness` de ancho y
`slot_alto + 2 × thickness` de alto. Cada pieza se dibuja a `thickness` px
de grosor.

---

## 5. Borde interno de cada slot (`[slot_frame]`, opcional)

Un segundo marco 9-slice, más chico, dibujado **dentro de cada uno de los
9 slots** (encima del cuadro vacío). Usa el mismo motor y los mismos
campos que `[frame]` (`border_cells`, `thickness`, las 8 celdas), pero se
ubica dentro del rect de cada slot en vez de alrededor de la barra. Es
**opcional**: sin la sección, los slots no llevan borde interno.

```toml
[slot_frame]
border_cells = 1       # cada pieza es 1 celda (borde interno fino)
thickness    = 4       # grosor del borde interno en px (fino)
top_left     = [8, 8]
top_right    = [11, 8]
bottom_left  = [8, 11]
bottom_right = [11, 11]
top          = [9, 8]
bottom       = [9, 11]
left         = [8, 10]
right        = [11, 10]
```

Mismos campos y semántica que `[frame]` (no acepta `inset`). El `thickness`
del borde interno también escala con `ui_scale`, así el conjunto completo
—barra, marco y bordes internos— se achica o agranda junto.

---

## 6. Cómo cambiar la barra

- **Mover o reescalar:** editá `scale`, `margin` o `anchor` en `[bar]`.
- **Achicar/agrandar todo junto:** cambiá `ui_scale` en `[bar]`.
- **Separar los cuadros:** subí `gap` (en píxeles).
- **Usar otro cuadro de slot:** cambiá `cell` en `[slot]` por otra celda
  de la hoja (usá la herramienta de inspección para encontrar sus
  índices).
- **Quitar/cambiar el borde interno:** editá o borrá `[slot_frame]`.
- **Otra hoja:** cambiá `image`/`cell` en `[sheet]`.

Guardá y volvé a correr el cliente. **No hace falta recompilar** código
C++.
