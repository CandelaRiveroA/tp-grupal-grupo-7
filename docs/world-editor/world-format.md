# Formato de mundo (`world.toml`)

Un mundo es un único archivo TOML que describe todos sus mapas, el terreno,
la colisión y los puntos de juego (objetos, marcadores y portales). El
modelo no fija una ruta: `WorldSerializer::loadFromFile` y
`WorldSerializer::saveToFile` reciben el path; el editor de la Fase 2
decidirá el directorio de mundos.

Lo consume el modelo de datos puro (`worldmodel::WorldSerializer`) y lo
usan el editor, la fábrica de mundos y los tests. No depende de Qt, SDL ni
del servidor.

> **Alcance:** este archivo es el **modelo de edición**, no una capa de
> presentación. Los tilesets indican qué imagen y qué celda usar para
> dibujar el terreno, pero las colisiones y los puntos de juego son datos
> del mundo. Cambiar un tileset no cambia las reglas; cambiar `collision`
> sí.

La validación estructural (campos y tipos) la hace el reader. Las reglas
de contenido (mapas obligatorios, objetos, portales, caminabilidad) las
hace `WorldValidator`; un archivo puede cargar y aun así tener issues.

---

## 1. Versión

```toml
format_version = 1
```

| Campo | Tipo | Obligatorio | Descripción |
|---|---|---|---|
| `format_version` | int | sí | Versión del formato. Debe ser exactamente `1`. |

---

## 2. Mundo

```toml
[world]
name = "Primavera"
```

| Campo | Tipo | Obligatorio | Descripción |
|---|---|---|---|
| `name` | string | sí | Nombre visible y, a la vez, nombre del archivo (`<name>.toml`). Validación semántica: no vacío, sin `/`, distinto de `.` y `..`. |

No hay campo `description` en el modelo actual.

---

## 3. Tilesets

Un tileset es una imagen y el tamaño de celda de su grilla, en píxeles.
Se define como `[tilesets.<nombre>]`; el nombre es el que usan los
encodings de tiles.

```toml
[tilesets.outdoors]
image = "Farm/spring_outdoors.png"
cell = [16, 16]
```

| Campo | Tipo | Obligatorio | Descripción |
|---|---|---|---|
| `image` | string | sí | Ruta de la imagen, relativa al directorio de assets. |
| `cell` | `[int, int]` | sí | `[ancho, alto]` de la celda en píxeles; ambos positivos. |

El lector acepta varios tilesets y no exige que todos se usen. Un
`ground`, `paths` u objeto que referencie un tileset inexistente es un
error de lectura (`FormatError`).

---

## 4. Mapas

Cada mapa es una tabla `[maps.<id>]`. El **id es la clave de la tabla**;
no existe un campo `id` dentro del mapa. El reader asigna ese id al
modelo y el writer lo vuelve a emitir como clave.

```toml
[maps.farm]
kind = "farm"
width = 3
height = 2
tile_size = 16
base_collision = [false, false, false, false, false, false]
collision = [true, true, false, false, false, false]
ground = [
  "outdoors:0,0", "outdoors:0,0", "outdoors:0,0",
  "outdoors:0,0", "outdoors:1,0", "outdoors:0,0",
]
paths = [
  "", "", "",
  "", "outdoors:5,0", "",
]
```

| Campo | Tipo | Obligatorio | Descripción |
|---|---|---|---|
| `kind` | string | sí | `farm`, `town`, `cabin`, `market`, `community_center` o `dungeon`. |
| `level` | int | solo `dungeon` | Nivel 1..10. En los mapas obligatorios debe coincidir con el id (`dungeon_3` → `3`). El reader exige `>= 0`; el validador exige el rango. |
| `width` | int | sí | Ancho en celdas; positivo. |
| `height` | int | sí | Alto en celdas; positivo. |
| `tile_size` | int | sí | Tamaño de celda en píxeles; positivo. Es escala de edición/dibujo. |
| `base_collision` | `[bool]` | sí | Colisión base del terreno. Exactamente `width * height` elementos. |
| `collision` | `[bool]` | sí | Colisión final. Exactamente `width * height` elementos. |
| `ground` | `[string]` | sí | Terreno visual. Exactamente `width * height` elementos. |
| `paths` | `[string]` | no | Caminos visuales. Si se omite, queda `width * height` celdas vacías. |
| `objects` | array de tablas | no | Ver sección 5. |
| `markers` | array de tablas | no | Ver sección 6. |
| `portals` | array de tablas | no | Ver sección 7. |

`true` significa **bloqueado** y `false` **caminable**. `paths` no abre ni
cierra paso; es solo presentación dentro del editor.

### Los 15 mapas obligatorios

| Id | `kind` | Contenido mínimo |
|---|---|---|
| `farm` | `farm` | Una cabaña, al menos un `player_spawn`, `town_entrance` y `dungeon_entrance`. |
| `town` | `town` | Un mercado, un centro comunitario y `farm_entrance`. |
| `cabin` | `cabin` | Al menos un `bed` y un `interior_exit`. |
| `market` | `market` | Marnie, Clint, Pierre y Robin (un marcador de cada uno) y un `interior_exit`. |
| `community_center` | `community_center` | Un `interior_exit`. |
| `dungeon_1` … `dungeon_10` | `dungeon` | Nivel correcto y los portales de la topología. |

### Topología de la mazmorra

- `farm.dungeon_entrance` → `dungeon_1`.
- Cada `dungeon_n` tiene un `dungeon_exit` → `farm`.
- Cada `dungeon_n` con `n < 10` tiene un `stairs_down` → `dungeon_{n+1}`.
- `dungeon_10` **no** tiene `stairs_down`.
- No se exige reciprocidad entre escaleras.

### Mapas extra

Cualquier id que no sea uno de los 15 reservados y cumpla
`[a-z0-9_-]+` (sin puntos) es un mapa extra. Reutiliza un `kind`
existente; no hay `MapKind::Custom`. Por ejemplo, una segunda cueva es un
mapa extra con `kind = "dungeon"`.

Un mapa extra:

- no exige objetos, marcadores ni portales;
- admite cualquier marcador;
- se conecta con portales `link` (sección 7);
- no necesita ser alcanzable: el validador no hace flood fill.

Los mapas extra con `kind = "dungeon"` aceptan cualquier `level`; los
extra con otro `kind` deben usar `level = 0` (o omitirlo).

---

## 5. Objetos

Los objetos son muebles o edificios. Su footprint puede bloquear celdas.
Se escriben como array de tablas dentro del mapa:

```toml
[[maps.farm.objects]]
role = "cabin"
tile = "outdoors:10,0"
cell = [0, 0]
size = [2, 1]
blocked = true
door = [0, 1]
```

| Campo | Tipo | Obligatorio | Descripción |
|---|---|---|---|
| `role` | string | sí | `cabin`, `market`, `community_center` o `decoration`. |
| `tile` | string | sí | Encoding no vacío `"tileset:col,row"`; el tileset debe existir. |
| `cell` | `[int, int]` | sí | Celda superior izquierda `[columna, fila]`. |
| `size` | `[int, int]` | no | `[ancho, alto]` en celdas. Default `[1, 1]`. |
| `blocked` | bool | no | Si es `true`, todo el footprint bloquea. Default `false`. |
| `door` | `[int, int]` | no | Celda absoluta de la puerta dentro de este mapa. |

Reglas semánticas:

- `farm` exige exactamente una `cabin`; `town` exige exactamente un
  `market` y un `community_center`.
- Las cabañas, mercados y centros comunitarios deben tener `door`, y esa
  celda debe estar dentro del mapa y ser caminable.
- El footprint completo debe quedar dentro del mapa.
- Un objeto `blocked` aporta `true` a su footprint en `collision`.

---

## 6. Marcadores

Los marcadores son puntos de juego, no tiles. Se escriben como array de
tablas:

```toml
[[maps.farm.markers]]
kind = "player_spawn"
cell = [2, 1]
```

| Campo | Tipo | Obligatorio | Descripción |
|---|---|---|---|
| `kind` | string | sí | Ver tabla siguiente. |
| `cell` | `[int, int]` | sí | Celda `[columna, fila]`. |

| `kind` | Dónde se exige |
|---|---|
| `player_spawn` | `farm`, al menos uno. |
| `bed` | `cabin`, al menos uno. |
| `merchant_marnie` | `market`, exactamente uno. |
| `merchant_clint` | `market`, exactamente uno. |
| `merchant_pierre` | `market`, exactamente uno. |
| `merchant_robin` | `market`, exactamente uno. |

En los 15 mapas obligatorios, un marcador de otro tipo es
`UnexpectedMarker`. Los mapas extra aceptan cualquier marcador. Todo
marcador debe estar dentro del mapa y en una celda caminable. Spawn y
camas pueden repetirse; los comerciantes de `market`, no.

---

## 7. Portales

Los portales conectan mapas. Se escriben como array de tablas:

```toml
[[maps.farm.portals]]
kind = "town_entrance"
cell = [1, 1]
target_map = "town"
target_cell = [0, 1]
```

| Campo | Tipo | Obligatorio | Descripción |
|---|---|---|---|
| `kind` | string | sí | Ver tabla siguiente. |
| `cell` | `[int, int]` | sí | Celda de origen `[columna, fila]`. |
| `target_map` | string | sí | Id de un mapa existente; no vacío. |
| `target_cell` | `[int, int]` | sí | Celda de destino `[columna, fila]`. |

| `kind` | Mapas donde se admite | `target_map` esperado |
|---|---|---|
| `town_entrance` | `farm` | `town` |
| `farm_entrance` | `town` | `farm` |
| `dungeon_entrance` | `farm` | `dungeon_1` |
| `dungeon_exit` | `dungeon_1` … `dungeon_10` | `farm` |
| `stairs_down` | `dungeon_1` … `dungeon_9` | `dungeon_{n+1}` |
| `interior_exit` | `cabin`, `market`, `community_center` | `farm` desde cabaña; `town` desde mercado y centro |
| `link` | cualquier mapa | cualquier mapa existente |

Reglas semánticas:

- `farm` exige un `town_entrance` y un `dungeon_entrance`.
- `town` exige un `farm_entrance`.
- `cabin`, `market` y `community_center` exigen un `interior_exit`.
- Cada `dungeon_n` exige un `dungeon_exit`; los niveles 1..9 exigen
  además un `stairs_down`.
- Los portales fijos son únicos en su mapa. Puede haber cualquier
  cantidad de `link`.
- `interior_exit` debe apuntar exactamente a la celda `door` del objeto
  correspondiente en el mapa destino.
- Origen y destino deben estar dentro de sus mapas y ser caminables.
- Los portales son dirigidos: para ida y vuelta se colocan dos `link`.

---

## 8. Colisión

`base_collision`, `collision`, `ground` y `paths` son arrays planos de
`width * height` elementos, ordenados por filas:

```
índice = row * width + column
```

Ejemplo 3×2:

```
width = 3, height = 2

base_collision = [
  false, false, false,   # row 0 → índices 0..2
  false, false, false,   # row 1 → índices 3..5
]

celda (column=1, row=1) → índice 1*3+1 = 4
```

- `base_collision` es la colisión del terreno/preset, sin objetos.
- `collision` es la colisión final que consume el juego.
- `collision` debe ser igual a `base_collision` más los footprints de los
  objetos `blocked`:

```
collision = base_collision OR footprint(objeto 1) OR footprint(objeto 2) ...
```

- Al mover, borrar o cambiar `blocked` de un objeto hay que recomponer
  `collision`. En el modelo se hace con `Map::rebuildCollision()`; al
  editar el TOML a mano hay que recalcular el array.
- `ground` y `paths` son visuales: no participan de la colisión.

### Encoding de tiles

```
""                 → celda vacía
"outdoors:3,2"     → tileset "outdoors", columna 3, fila 2
```

Se exige exactamente un `:` y una `,`, tileset no vacío, enteros no
negativos y sin espacios. El tileset debe existir en `[tilesets]`.

---

## 9. Ejemplo completo

Ejemplo recortado a dos mapas para que se lea completo. Un mundo real
repite `[maps.<id>]` para los 15 mapas obligatorios; la tabla de la
sección 4 indica qué contenido mínimo necesita cada uno.

```toml
format_version = 1

[world]
name = "Primavera"

[tilesets.outdoors]
image = "Farm/spring_outdoors.png"
cell = [16, 16]

[tilesets.interiors]
image = "Interiors/cabin.png"
cell = [16, 16]

[maps.farm]
kind = "farm"
width = 3
height = 2
tile_size = 16
base_collision = [false, false, false, false, false, false]
collision = [true, true, false, false, false, false]
ground = [
  "outdoors:0,0", "outdoors:0,0", "outdoors:0,0",
  "outdoors:0,0", "outdoors:1,0", "outdoors:0,0",
]
paths = [
  "", "", "",
  "", "outdoors:5,0", "",
]

[[maps.farm.objects]]
role = "cabin"
tile = "outdoors:10,0"
cell = [0, 0]
size = [2, 1]
blocked = true
door = [0, 1]

[[maps.farm.markers]]
kind = "player_spawn"
cell = [2, 1]

[[maps.farm.portals]]
kind = "town_entrance"
cell = [1, 1]
target_map = "town"
target_cell = [1, 1]

[[maps.farm.portals]]
kind = "dungeon_entrance"
cell = [2, 0]
target_map = "dungeon_1"
target_cell = [0, 0]

[[maps.farm.portals]]
kind = "link"
cell = [1, 0]
target_map = "cave"
target_cell = [0, 0]

[maps.town]
kind = "town"
width = 3
height = 2
tile_size = 16
base_collision = [false, false, false, false, false, false]
collision = [true, false, true, false, false, false]
ground = [
  "outdoors:0,0", "outdoors:0,0", "outdoors:0,0",
  "outdoors:0,0", "outdoors:0,0", "outdoors:0,0",
]

[[maps.town.objects]]
role = "market"
tile = "outdoors:11,0"
cell = [0, 0]
size = [1, 1]
blocked = true
door = [0, 1]

[[maps.town.objects]]
role = "community_center"
tile = "outdoors:12,0"
cell = [2, 0]
size = [1, 1]
blocked = true
door = [2, 1]

[[maps.town.markers]]
kind = "merchant_marnie"
cell = [1, 1]

[[maps.town.markers]]
kind = "merchant_clint"
cell = [1, 0]

[[maps.town.markers]]
kind = "merchant_pierre"
cell = [0, 1]

[[maps.town.markers]]
kind = "merchant_robin"
cell = [2, 1]

[[maps.town.portals]]
kind = "farm_entrance"
cell = [1, 1]
target_map = "farm"
target_cell = [1, 1]

[maps.dungeon_1]
kind = "dungeon"
level = 1
width = 2
height = 2
tile_size = 16
base_collision = [false, false, false, false]
collision = [false, false, false, false]
ground = [
  "interiors:0,0", "interiors:0,0",
  "interiors:0,0", "interiors:0,0",
]

[[maps.dungeon_1.portals]]
kind = "dungeon_exit"
cell = [0, 0]
target_map = "farm"
target_cell = [2, 0]

[[maps.dungeon_1.portals]]
kind = "stairs_down"
cell = [1, 1]
target_map = "dungeon_2"
target_cell = [0, 0]

[maps.cave]
kind = "dungeon"
level = 5
width = 1
height = 1
tile_size = 16
base_collision = [false]
collision = [false]
ground = ["interiors:0,0"]

[[maps.cave.portals]]
kind = "link"
cell = [0, 0]
target_map = "farm"
target_cell = [1, 0]
```

> El ejemplo anterior es **estructural**: muestra todas las secciones y
> pasa el reader, pero no `WorldValidator` porque faltan los otros mapas
> obligatorios. Para un mundo válido, copiar la tabla de la sección 4 y
> agregar los mapas restantes con su contenido mínimo.

---

## 10. Recetas

### Agregar un objeto

1. Agregar un bloque `[[maps.<id>.objects]]` con `role`, `tile`, `cell`,
   `size`, `blocked` y, si es edificio, `door`.
2. Si `blocked = true`, recomponer `collision` para incluir el footprint.
3. Verificar que la puerta sea caminable y que el footprint quede dentro
   del mapa.

### Agregar un marcador

1. Agregar `[[maps.<id>.markers]]` con `kind` y `cell`.
2. Usar una celda dentro del mapa y caminable.
3. Respetar las reglas de la sección 6: spawn en `farm`, cama en
   `cabin`, comerciantes únicos en `market`.

### Agregar un portal

1. Agregar `[[maps.<id>.portals]]` con `kind`, `cell`, `target_map` y
   `target_cell`.
2. Verificar que `target_map` exista y que ambas celdas sean caminables.
3. Para un `link` de ida y vuelta, agregar también el portal inverso en
   el mapa destino.
4. Para un `interior_exit`, apuntar a la celda `door` del edificio en el
   mapa destino.

### Agregar un mapa extra y conectarlo con `link`

1. Copiar el terreno de un escenario como `[maps.<id_nuevo>]`, con un id
   que cumpla `[a-z0-9_-]+` y no sea uno de los 15 reservados.
2. Mantener el `kind` del escenario; si es `dungeon`, elegir cualquier
   `level`.
3. Agregar un `link` desde un mapa existente hacia el nuevo y, si se
   quiere volver, otro desde el nuevo hacia el origen.
4. Verificar que los destinos existan y sean caminables.

Programáticamente, `WorldFactory::addExtraMap` copia el escenario y deja
el mapa sin objetos, marcadores ni portales; después se agregan los
`link` al `World`.

---

## 11. Notas

- Los ids de mapa son **bare**: `[a-z0-9_-]+`, sin puntos.
- Los comentarios y el orden original **no se preservan** al guardar. El
  writer ordena mapas y tilesets alfabéticamente y omite `paths` si todas
  sus celdas están vacías, y omite `objects`, `markers` o `portals` si
  están vacíos.
- El reader ignora campos que no conoce y no valida reglas de contenido;
  para eso está `WorldValidator`.
- Un mundo puede cargar correctamente y aun así tener issues. El orden de
  validación no es el orden del archivo.
- Cada mapa del mundo es independiente: no hay herencia entre mapas ni
  referencias a escenarios una vez creado el mundo.
