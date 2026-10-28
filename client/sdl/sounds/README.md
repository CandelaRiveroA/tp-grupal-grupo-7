# Sonidos — estructura y convención

> Chequeá esto antes de subir cualquier audio. La checklist de **qué** sonidos faltan está en
> [`sonidos-necesarios.md`](sonidos-necesarios.md); este archivo es sobre **dónde va y cómo se nombra**.

## Estructura

```
client/sdl/sounds/
├── music/                  # fondos largos → Mix_Music (stream del disco)
├── sfx/                    # efectos cortos → Mix_Chunk (cargan entero en RAM)
│   ├── movement/           #   pasos, rodar, saltar
│   ├── tools/              #   pico, regadera, pala, espada
│   ├── player/             #   energía, vida, despertar, inconsciente
│   ├── farm/               #   cultivos y animales
│   ├── mining/             #   caverna: minerales, escaleras, trampas
│   ├── combat/             #   enemigos: spawn, golpe, muerte
│   ├── ui/                 #   botones, ventanas, mapa, errores
│   └── multiplayer/        #   chat, joins, regalos, código
├── *.wav / *.mp3           # sueltos = pendientes de convertir/categorizar
└── sonidos-necesarios.md   # checklist de faltantes
```

Las carpetas con solo `.gitkeep` están vacías a propósito: git no trackea directorios vacíos,
el `.gitkeep` es un placeholder sin contenido.

## Regla de oro: `music/` vs `sfx/`

No es estética, es **memoria**. Son dos APIs distintas de `SDL2_mixer`:

| Carpeta | API | Memoria | Uso |
|---|---|---|---|
| `music/` | `Mix_Music` + `Mix_PlayMusic` | Stream del disco, **una a la vez** | fondos de zona/pantalla, loops largos |
| `sfx/` | `Mix_Chunk` + `Mix_PlayChannel` | **Se carga entero en RAM**, muchos a la vez | cualquier efecto |

Un `.ogg` de 1.8 MB cargado como `Mix_Chunk` es exactamente lo que pide evitar el enunciado
("la menor memoria posible para no sobrecargar la computadora"). Si dudás entre carpeta y API:
**largo y de fondo = `music/`, corto y repetido = `sfx/`**.

El enunciado además pide **control de volumen de sonido y de música por separado**: los dos
directorios mapean 1:1 con esos dos canales.

## Formato

- **`.ogg` (Vorbis)** para todo. Comprimido, un solo decoder, sirve igual para `Mix_Music` y
  `Mix_Chunk`. Un `.wav` de 8 MB ronda los 800 KB en `.ogg`.
- **`.mp3` y `.wav` sueltos en la raíz**: son material en bruto pendiente. No subir así al repo.
- Convertir antes de categorizar:

```bash
ffmpeg -i archivo.wav -c:a libvorbis -q:a 128 archivo.ogg   # sfx (corta, calidad alta)
ffmpeg -i archivo.mp3 -c:a libvorbis -q:a 4 archivo.ogg     # música (q4 ≈ 128k)
```

## Nombres

`snake_case`, en inglés, sin acentos ni espacios, **sin prefijo de carpeta**
(`sfx/movement/sfx_step.ogg` sería repetir `sfx`).

- **música** → el lugar o momento: `menu_theme.ogg`, `character_select.ogg`,
  `farm_day.ogg`, `dungeon_ambient.ogg`
- **sfx** → la acción concreta: `step.ogg`, `pickaxe_hit.ogg`, `coin_pickup.ogg`,
  `cow_moo.ogg`, `ui_click.ogg`

Si un mismo evento tiene variantes, sufijarlas: `step_1.ogg`, `step_2.ogg`, `cow_moo_1.ogg`.

## Antes de subir — checklist

- [ ] ¿Es **largo y de fondo**? → `music/`. ¿Es **corto y repetible**? → `sfx/<categoría>/`.
- [ ] ¿Está en **`.ogg`**? Si no, convertir y borrar el original pesado.
- [ ] ¿El nombre está en **inglés**, `snake_case`, sin prefijo redundante?
- [ ] ¿Pesa menos de ~500 KB? Si es más y es `music/`, igual está bien (es stream).
      Si es más y es `sfx/`, recortalo: todo entra entero en RAM.
- [ ] ¿Quedó el `.wav`/`.mp3` original? **No** subirlo, solo el `.ogg`.
- [ ] ¿Tildaste el ítem en `sonidos-necesarios.md`?

## Estado del build

`SDL2_mixer` se baja por `FetchContent` en la raíz y **ya está linkeado** al cliente:

```cmake
target_link_libraries(taller_client PRIVATE SDL2_mixer::SDL2_mixer)
```

El soporte OGG/Vorbis viene compilado con el decoder embebido `stb_vorbis`
(`-DMUSIC_OGG -DOGG_USE_STB`), así que **no** requiere `libvorbis`/`libogg` del sistema. El
mensaje `SDL2_mixer support disabled` que aparece al configurar es del wrapper opcional de
SDL2pp (que no usamos): el audio del cliente usa la API C de `SDL2_mixer` encerrada en
`sdl/audio/AudioPlayer`.

Los archivos de esta carpeta se copian junto al binario en cada build (a `build/sounds/`); el
cliente los resuelve con `core::soundPath("sfx/...")`. Para sumar un sonido nuevo alcanza con
dejarlo acá y agregar una fila al catálogo en `sdl/audio/AudioCatalog.h`.
