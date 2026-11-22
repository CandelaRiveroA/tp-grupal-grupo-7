# Formato de escenario (`scenario.toml`)

Un escenario es un **preset de terreno**: tilesets, grilla de terreno y
colisión base. Se usa para crear los mapas de un mundo, pero no es un
mundo: no tiene objetos, marcadores, portales ni puntos de juego.

Los escenarios viven en un directorio de archivos `.toml`.
`ScenarioLibraryLoader::loadFromDirectory` carga todos los `.toml` en
orden alfabético y los entrega como `ScenarioLibrary`; cada archivo se
lee con `ScenarioSerializer::loadFromFile`. El modelo no fija una ruta:
los presets reales los aporta el usuario; para tests se usan, por
ejemplo, `tests/fixtures/scenarios/`.

> **Alcance:** este archivo es el **modelo de edición**, no una capa de
> presentación. `image` y `cell` describen cómo dibujar el terreno en el
> editor. La colisión y el terreno son datos del preset; el cliente
> dibuja lo que indique el mundo que se derive de él.

La validación estructural la hace el reader. La identidad del escenario
en la biblioteca (id no vacío y único) la valida `ScenarioLibrary::add`.

---

## 1. Versión

```toml
format_version = 1
```

| Campo | Tipo | Obligatorio | Descripción |
|---|---|---|---|
| `format_version` | int | sí | Versión del formato. Debe ser exactamente `1`. |

---

## 2. Escenario

```toml
[scenario]
id = "farm_a"
kind = "farm"
width = 2
height = 2
tile_size = 16
collision = [false, false, false, true]
ground = ["outdoors:0,0", "outdoors:1,0", "outdoors:0,0", ""]
```

| Campo | Tipo | Obligatorio | Descripción |
|---|---|---|---|
| `id` | string | sí | Identificador del preset. No vacío y único dentro de la biblioteca. |
| `kind` | string | sí | `farm`, `town`, `cabin`, `market`, `community_center` o `dungeon`. |
| `width` | int | sí | Ancho en celdas; positivo. |
| `height` | int | sí | Alto en celdas; positivo. |
| `tile_size` | int | sí | Tamaño de celda en píxeles; positivo. |
| `collision` | `[bool]` | sí | Colisión base del terreno. Exactamente `width * height` elementos. |
| `ground` | `[string]` | sí | Terreno visual. Exactamente `width * height` elementos. |

No hay `level`: un escenario `dungeon` no pertenece a un nivel. El nivel
se asigna al crear el mapa (`dungeon_1` → `1`, etc.) o al agregarlo como
mapa extra.

Un escenario **no** tiene `paths`, `objects`, `markers` ni `portals`. Al
copiar el escenario a un mundo:

| Escenario | Mapa del mundo |
|---|---|
| `ground` | `ground` |
| `collision` | `baseCollision` |
| `tilesets` | se agregan a `World.tilesets` |
| — | `paths` vacío |
| — | `objects`, `markers`, `portals` vacíos |

El mundo guarda una copia: editar el preset después no cambia los mundos
ya creados.

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

Puede haber varios tilesets y no es obligatorio que todos se usen. Un
tile de `ground` que referencie un tileset inexistente es un error de
lectura.

---

## 4. Colisión y terreno

`collision` y `ground` son arrays planos de `width * height` elementos,
ordenados por filas:

```
índice = row * width + column
```

Ejemplo 2×2:

```
width = 2, height = 2

collision = [
  false, false,   # row 0 → índices 0..1
  false, true,    # row 1 → índices 2..3
]

celda (column=1, row=1) → índice 1*2+1 = 3 → true → bloqueada
```

- `true` significa **bloqueado** y `false` **caminable**.
- `ground` usa el encoding de tiles de la sección 5.
- La colisión del escenario es la **base**: al crear el mapa, los
  objetos `blocked` agregan sus footprints a la colisión final. El
  escenario no conoce objetos.

---

## 5. Encoding de tiles

```
""                 → celda vacía
"outdoors:3,2"     → tileset "outdoors", columna 3, fila 2
```

Se exige exactamente un `:` y una `,`, tileset no vacío, enteros no
negativos y sin espacios. El tileset debe existir en `[tilesets]`.

---

## 6. Ejemplo completo

```toml
format_version = 1

[scenario]
id = "farm_a"
kind = "farm"
width = 2
height = 2
tile_size = 16
collision = [false, false, false, true]
ground = [
  "outdoors:0,0", "outdoors:1,0",
  "outdoors:0,0", "",
]

[tilesets.outdoors]
image = "Farm/spring_outdoors.png"
cell = [16, 16]
```

Este archivo es válido y sobrevive un round-trip. Al usarlo para crear el
mapa `farm`, ese mapa empieza con el terreno y la colisión base del
preset, sin cabaña, spawn ni portales.

---

## 7. Recetas

### Agregar un escenario a la biblioteca

1. Crear un `.toml` nuevo en el directorio de escenarios.
2. Elegir un `id` no vacío y que no exista en otro archivo del directorio.
3. Elegir el `kind` y definir `width`, `height`, `tile_size`, `ground`,
   `collision` y al menos un tileset.
4. Cargar el directorio con `ScenarioLibraryLoader::loadFromDirectory`.

La biblioteca puede tener varios escenarios del mismo `kind`; no hay
límite. Un id repetido o vacío hace que `ScenarioLibrary::add` lance
`FormatError` con path `scenario`.

### Usar un escenario en un mundo

- Para uno de los 15 mapas obligatorios, elegir el id en
  `ScenarioChoice` (`farm`, `town`, `cabin`, `market`,
  `communityCenter`, `dungeons`) y llamar a `WorldFactory::makeDefault`.
- Para un mapa extra, copiar cualquier escenario con
  `WorldFactory::addExtraMap`; el mapa conserva el `kind` del preset y
  queda sin puntos. Después se conecta con portales `link`.

### Cambiar el terreno de un mundo existente

No alcanza con editar el preset: el mundo ya tiene una copia. Hay que
editar el mapa dentro del mundo (o volver a crearlo) y recomponer su
`collision`.

---

## 8. Notas

- Solo se cargan archivos con extensión `.toml`; el orden de carga es
  alfabético por path.
- Los comentarios y el orden original **no se preservan** al guardar. El
  writer ordena los tilesets alfabéticamente.
- El id del escenario es libre; se recomienda usar el mismo estilo bare
  que los ids de mapa (`[a-z0-9_-]+`) para evitar problemas al
  referenciarlo.
- Un escenario es solo terreno. Los objetos, marcadores y portales se
  agregan al mundo, no al preset.
