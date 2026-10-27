# Formato de personajes (`characters.toml`)

La apariencia de cada criatura se describe como datos en
`client/sdl/assets/characters.toml`. El código de dibujo y de animación es
**genérico**: lee este archivo y dibuja/anima lo que indique, sin tener
las poses ni las velocidades cableadas en C++. Para cambiar cómo se ve o a
qué ritmo anda una criatura se edita este archivo; **no hace falta
recompilar** el cliente.

> Igual que `scenes.toml` con el decorado, esto es **presentación**: cómo
> se *dibuja* y *anima* cada criatura en el cliente. No describe reglas de
> juego (velocidad real, colisiones): eso es autoridad del servidor. Que
> acá un paso dure 24 ticks es "animá la zancada en este tiempo", no "el
> jugador se mueve a esta velocidad en el mundo".

El archivo lo carga `world::CharacterCatalogLoader` hacia un
`world::CharacterCatalog` (datos puros, sin SDL), y el `sdl::Renderer` lo
consume. La clave de cada criatura es un `world::CreatureKind` (un enum en
C++): el loader valida que haya una tabla por cada valor del enum.

---

## 1. Una criatura

Cada criatura es una tabla `[characters.<kind>]`. Hoy existe
`[characters.player]`. El nombre (`player`) corresponde a un valor del
enum `world::CreatureKind`.

```toml
[characters.player]
sheet   = "Players/main/Character_Base.png"   # hoja (relativa a assets)
grid    = [16, 16]           # celda de ESA hoja [ancho, alto] en px
drawn   = [32, 64]           # tamaño dibujado del CUERPO [ancho, alto]
cadence = { step = 24, frame = 12 }
```

| Campo     | Tipo              | Obligatorio | Descripción                                        |
|-----------|-------------------|-------------|----------------------------------------------------|
| `sheet`   | string            | sí          | Ruta de la hoja de sprites (relativa a assets)     |
| `grid`    | `[int, int]`      | sí          | Tamaño de celda de la hoja `[ancho, alto]` (px)    |
| `drawn`   | `[int, int]`      | sí          | Tamaño dibujado del cuerpo `[ancho, alto]` (px)    |
| `cadence` | tabla inline      | sí          | Ritmo de la caminata (ver abajo)                   |

### `cadence`

```toml
cadence = { step = 24, frame = 12 }
```

| Campo   | Tipo | Obligatorio | Descripción                                   |
|---------|------|-------------|-----------------------------------------------|
| `step`  | int  | sí          | Ticks que tarda en pasar de una celda a otra  |
| `frame` | int  | sí          | Ticks por cuadro DEFAULT de las acciones      |

**Invariante** (la valida el loader SOLO contra `walk`, usando el ritmo
PROPIO de walk —su `frame` de acción, por defecto el de la cadencia—):
`step == frame_de_walk * (cantidad de cuadros de walk)`. Así un paso
dura exactamente un ciclo completo de piernas. Además `frame > 0`,
`step > 0` y `step % frame == 0` (la cadencia por sí sola también tiene
que ser coherente).

`cadence.frame` es además el ritmo por defecto de TODAS las acciones:
la que no declara su propio `frame` (§3) usa este. Así caminar queda
con el ritmo de la cadencia mientras una one-shot como `use_tool` va
más rápida, sin que una condicione a la otra.

---

## 2. Direcciones

Cada criatura tiene cuatro subtablas de dirección: `front`, `back`,
`left`, `right` (mirando a la cámara, de espaldas, a la izquierda y a la
derecha). Las cuatro son obligatorias.

```toml
  [characters.player.front]
  mirrored = false
    [characters.player.front.idle]
    loops  = false
    frames = [ [ [0, 0, 1, 2], [6, 1, 1, 1] ] ]
    [characters.player.front.walk]
    loops  = true
    frames = [
      [ [1, 0, 1, 2], [7, 1, 1, 1] ],
      [ [2, 0, 1, 2], [8, 1, 1, 1] ],
    ]
```

| Campo      | Tipo         | Obligatorio | Descripción                                 |
|------------|--------------|-------------|---------------------------------------------|
| `mirrored` | bool         | sí          | Si se dibuja espejada horizontalmente       |
| `idle`     | acción       | **sí**      | Pose quieta (ver §3)                        |
| `walk`     | acción       | **sí**      | Caminata (además dispara el sonido de paso) |
| otras      | acción       | no          | Acciones opcionales, p. ej. `use_tool`      |

### Espejado: izquierda y derecha

El sprite lateral de la hoja mira hacia la **derecha**. Por eso `right`
usa esas celdas tal cual (`mirrored = false`) y `left` usa las **mismas**
celdas volteadas (`mirrored = true`): no se duplican sprites. En el
archivo las dos direcciones repiten las mismas celdas a propósito,
cambiando solo el flag `mirrored`.

---

## 3. Acciones, cuadros y capas

### Una acción

Cada acción es una subtabla con el nombre de la acción (clave TOML →
`world::ActionKind`; los nombres conocidos viven en un único diccionario
del loader: sumar una acción es sumar una fila ahí):

```toml
    [characters.player.right.use_tool]
    loops  = false      # ¿cicla o se queda en el último cuadro?
    frame  = 8          # opcional: ritmo propio (si falta, cadence.frame)
    frames = [ ... ]    # lista de CUADROS en orden
```

| Campo    | Tipo          | Obligatorio | Descripción                                   |
|----------|---------------|-------------|-----------------------------------------------|
| `loops`  | bool          | sí          | `true` = cicla (walk); `false` = one-shot     |
| `frames` | lista de cuadros | sí       | Cada cuadro es una lista de capas             |
| `frame`  | int           | no          | Ritmo PROPIO: ticks por cuadro de ESTA acción; sin esto usa `cadence.frame` |

El ritmo es **por acción**: `frame` dice cada cuántos ticks avanza la
secuencia de ESTA acción, sin condicionar a las demás (caminar puede
seguir al ritmo de la cadencia mientras un one-shot va más rápido).
Validado `> 0`; la invariante de `step` SOLO se chequea contra `walk`
(§1).

- **Acciones obligatorias**: `idle` y `walk` (el loader las exige en las
  cuatro direcciones).
- **Acciones opcionales**: p. ej. `use_tool` (usar la herramienta activa:
  hoy picar). Si una dirección **no** tiene la acción cargada, pedirla no
  rompe: el render cae a `idle` (el personaje queda quieto).
- Una acción **one-shot** (`loops = false` con varios cuadros) se
  reproduce **una sola vez** —un toque de la tecla de usar = un golpe
  completo— y al terminar el estado de juego vuelve a `idle`. La ventana
  del golpe la lleva el cliente (`game/ToolUse`, construida desde los
  datos de la acción: cuadros × su ritmo), así ventana y animación no
  pueden desfasarse.

### Un cuadro: lista de capas

Un **cuadro** es una lista de **capas** en orden de dibujo: la primera se
pinta abajo, la última encima. Hoy son dos: **cuerpo** y **brazos**.

Una capa es:

| Formato                  | Significado                                        |
|--------------------------|----------------------------------------------------|
| `[col, row]`             | celda origen, tamaño **1×1** celdas (default)      |
| `[col, row, cols, rows]` | celda origen + **tamaño en celdas** de la capa     |

```toml
frames = [ [ [0, 0, 1, 2], [6, 1, 1, 1] ] ]
#            ^cuerpo          ^brazos
#            1x2 celdas       1x1 celda
```

El **tamaño es un dato de cada capa**, sin casos especiales: una capa
simplemente puede medir distinto que otra. En la hoja actual (celda
16×16) el cuerpo es `1×2` (16×32 px) y los brazos son `1×1` cuando están
bajos (front/right) o `1×2` cuando suben (back, y el brazo del picar).

Al dibujar, el factor de píxeles por celda sale de la **capa base** (el
cuerpo): `drawn / celdas_del_cuerpo`, y cada capa se dibuja a su tamaño
real escalado con ese factor. Una capa **más baja** que el cuadro del
personaje se **ancla abajo** (por eso los brazos 1×1 quedan en la mitad
inferior, no flotando).

Las celdas deben ser `col >= 0`, `row >= 0` y el tamaño `cols >= 1`,
`rows >= 1`.

---

## 4. Ejemplo completo: el jugador

```toml
[characters.player]
sheet   = "Players/main/Character_Base.png"
grid    = [16, 16]
drawn   = [32, 64]
cadence = { step = 24, frame = 12 }

  [characters.player.front]
  mirrored = false
    [characters.player.front.idle]
    loops  = false
    frames = [ [ [0, 0, 1, 2], [6, 1, 1, 1] ] ]
    [characters.player.front.walk]
    loops  = true
    frames = [
      [ [1, 0, 1, 2], [7, 1, 1, 1] ],
      [ [2, 0, 1, 2], [8, 1, 1, 1] ],
    ]

  [characters.player.back]
  mirrored = false
    [characters.player.back.idle]
    loops  = false
    frames = [ [ [0, 4, 1, 2], [6, 4, 1, 2] ] ]
    [characters.player.back.walk]
    loops  = true
    frames = [
      [ [1, 4, 1, 2], [7, 4, 1, 2] ],
      [ [2, 4, 1, 2], [8, 4, 1, 2] ],
    ]

  # Mismas celdas que right, espejada.
  [characters.player.left]
  mirrored = true
    [characters.player.left.idle]
    loops  = false
    frames = [ [ [0, 2, 1, 2], [6, 3, 1, 1] ] ]
    [characters.player.left.walk]
    loops  = true
    frames = [
      [ [1, 2, 1, 2], [7, 3, 1, 1] ],
      [ [2, 2, 1, 2], [8, 3, 1, 1] ],
    ]
    [characters.player.left.use_tool]
    loops  = false
    frame  = 6          # ritmo propio: 6 ticks por cuadro
    frames = [
      [ [0, 16, 1, 2], [6, 16, 1, 2] ],
      [ [1, 16, 1, 2], [7, 16, 1, 2] ],
      [ [2, 16, 1, 2], [8, 16, 1, 2] ],
      [ [3, 16, 1, 2], [9, 16, 1, 2] ],
      [ [4, 16, 1, 2], [10, 16, 1, 2] ],
    ]

  [characters.player.right]
  mirrored = false
    [characters.player.right.idle]
    loops  = false
    frames = [ [ [0, 2, 1, 2], [6, 3, 1, 1] ] ]
    [characters.player.right.walk]
    loops  = true
    frames = [
      [ [1, 2, 1, 2], [7, 3, 1, 1] ],
      [ [2, 2, 1, 2], [8, 3, 1, 1] ],
    ]
    # Picar: 5 cuadros, one-shot. Brazos 1x2 (levantados).
    # frame = 6: ritmo propio (5×6 = 30 ticks ≈ 500 ms), más ágil que caminar.
    [characters.player.right.use_tool]
    loops  = false
    frame  = 6
    frames = [
      [ [0, 16, 1, 2], [6, 16, 1, 2] ],
      [ [1, 16, 1, 2], [7, 16, 1, 2] ],
      [ [2, 16, 1, 2], [8, 16, 1, 2] ],
      [ [3, 16, 1, 2], [9, 16, 1, 2] ],
      [ [4, 16, 1, 2], [10, 16, 1, 2] ],
    ]
```

> Las cuatro direcciones (front, back, left, right) tienen `use_tool`
> cargada. Toda dirección que la declare debe usar la MISMA duración
> (cuadros × `frame`), porque el cliente arma una sola ventana de golpe
> y la reproduce para la dirección que sea: ver el test
> `RealFileToolUseDurationIsUniform`.

---

## 5. Cómo agregar una criatura nueva

1. Agregá su valor al enum `world::CreatureKind` (C++): es la clave
   tipeada con la que el render la pide.
2. Agregá su tabla `[characters.<kind>]` con `sheet`, `grid`, `drawn` y
   `cadence`, y las cuatro subtablas de dirección (`front`, `back`,
   `left`, `right`), cada una con `mirrored`, `idle` y `walk`.
3. Respetá la invariante de cadencia: `step == frame_de_walk *
   cuadros_de_walk` (donde `frame_de_walk` es el `frame` de la acción
   walk, o `cadence.frame` si walk no declara el suyo).
4. Si la criatura tiene acciones opcionales (`use_tool`, …), agregalas
   como subtablas más; en las direcciones sin esa acción simplemente no
   se reproduce.
5. Guardá. El loader valida que haya una tabla por cada valor del enum; si
   falta alguna, el cliente no arranca y reporta la criatura ausente.

### Ejemplo real: la gallina (`Farm/Chicken.png`)

Hoja relevada con `taller_tile_inspector`: 64×112 px con celdas de 16×16
(4 columnas × 7 filas). Las **cuatro direcciones traen su caminata
explícita** (filas 0–3: frente, derecha, espaldas, izquierda), así que no
se espeja nada (`mirrored = false` en todas). El cuerpo es **una capa de
1×1 celdas**: cada cuadro es el cuerpo entero.

```toml
[characters.chicken]
sheet   = "Farm/Chicken.png"
grid    = [16, 16]
drawn   = [32, 32]                    # el cuerpo entero se dibuja 32x32
cadence = { step = 32, frame = 8 }    # invariante: 32 == 8 * 4 (walk)

  [characters.chicken.front]
  mirrored = false
    [characters.chicken.front.idle]
    loops  = false
    frames = [ [ [0, 4] ] ]           # fila 4: pose "parada" de frente
    [characters.chicken.front.walk]
    loops  = true
    frames = [ [ [0, 0] ], [ [1, 0] ], [ [2, 0] ], [ [3, 0] ] ]  # fila 0

  [characters.chicken.right]
  mirrored = false                    # la hoja trae derecha explícita
    [characters.chicken.right.idle]
    loops  = false
    frames = [ [ [2, 4] ] ]           # fila 4: parada de lado derecho
    [characters.chicken.right.walk]
    loops  = true
    frames = [ [ [0, 1] ], [ [1, 1] ], [ [2, 1] ], [ [3, 1] ] ]  # fila 1

  [characters.chicken.back]
  mirrored = false
    [characters.chicken.back.idle]
    loops  = false
    frames = [ [ [0, 5] ] ]           # fila 5: parada de espaldas
    [characters.chicken.back.walk]
    loops  = true
    frames = [ [ [0, 2] ], [ [1, 2] ], [ [2, 2] ], [ [3, 2] ] ]  # fila 2

  [characters.chicken.left]
  mirrored = false                    # la hoja trae izquierda explícita
    [characters.chicken.left.idle]
    loops  = false
    frames = [ [ [2, 5] ] ]           # fila 5: parada de lado izquierdo
    [characters.chicken.left.walk]
    loops  = true
    frames = [ [ [0, 3] ], [ [1, 3] ], [ [2, 3] ], [ [3, 3] ] ]  # fila 3
```

- Cada capa acá es `[col, row]` de 1×1 (sin tamaño extra): una capa puede
  medir lo que quiera (§3).
- Quedan relevadas, como dato sin usar todavía: las poses **sentadas** de
  cada dirección (celdas (1,4), (3,4), (1,5), (3,5) de la hoja) y el
  **picoteo** de frente (fila 6, 4 cuadros). Sumarlas después es solo
  agregar subtablas de acción acá, sin tocar código.

> El audio (sonidos de paso) todavía vive en C++ (`sdl/audio/AudioCatalog.h`):
> son pocos sonidos y moverlos ensancharía esta migración sin cambiar nada.
> Queda como deuda conocida; si se suman muchos SFX, mismo tratamiento que
> acá (archivo + loader).
