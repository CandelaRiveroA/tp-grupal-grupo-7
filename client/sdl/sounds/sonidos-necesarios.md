# Checklist de sonidos — TP Stardew Valley (Grupo 7)

> **Fuente:** `Enunciado (1).md`, secciones **Música y sonidos**, **Configuración → in-game**,
> **Detalles técnicos pedidos #3** y todas las acciones descritas en el resto del documento.
>
> **Cómo usarlo:** cada checkbox es un audio a **buscar y descargar** a esta carpeta
> (`client/sdl/sounds/`). Cuando lo tengas, mové el archivo acá y tildá el ítem.
>
> **Leyenda:** 📌 = pedido literalmente en el enunciado · 💡 = inferido (la acción sí está
> descrita en el enunciado, su sonido la acompaña). En *itálica*, la búsqueda sugerida en
> inglés — los packs gratuitos están en inglés.

---

## ✅ Ya conseguido

- [x] Vaca (balido) → `Farm/Cow.mp3`
- [x] Gallina (cacareo **ambiental**, espontáneo cada ~20-45 s) →
  `sfx/farm/chicken_cluck.ogg` — **conectado al cliente**: cue
  `chicken_cluck` (`game/SoundCue.h`), eventos per-tick de la simulación
  (`LocalWorldView.soundEvents`) y gate por sala en `scenes.toml`
  (cada sala declara en `sounds = [...]` qué se oye mientras se la
  renderiza). El día que exista el granero, su sala declara el cue y listo.

---

## 1. 🎵 Música de fondo

### 📌 Zonas — recomendación explícita del enunciado ("música de fondo especial para el pueblo, la granja y la mazmorra")

- [ ] **Granja** — *cozy farm background music, loop*
- [ ] **Pueblo** (mercado + centro comunitario) — *small town market music, loop*
- [ ] **Mazmorra / caverna** — *dungeon ambient dark, loop*

### 💡 Momentos / pantallas (inferidos del recorrido del jugador)

- [ ] Pantalla principal / menú — *main menu theme*
- [ ] Creación de partida y personaje — *character creation music*
- [ ] **Victoria**: centro comunitario completo — *victory fanfare* (el enunciado pide "una pantalla especial felicitando al jugador por haber terminado el juego")
- [ ] Fin de día (2 am) / transición al día siguiente — *day end jingle*
- [ ] Caer inconsciente — *faint / game over sting*

---

## 2. 🧑 Acciones del jugador (herramientas y movimiento)

📌 El enunciado pide efectos "para cada acción del jugador (regar, atacar, moverse, cuidar animales, etc.)".

- [ ] **Moverse / caminar** (pasos) 📌 — *footsteps on grass/dirt*
- [ ] **Pico** golpeando piedra 📌 — *pickaxe hit rock*
- [ ] **Regadera** regando 📌 — *watering can pour*
- [ ] **Pala** cavando / tratando la tierra 💡 ("primero se debe tratar la tierra... se debe cavar el suelo") — *hoe till soil*
- [ ] **Espada**: swing al aire 💡 — *sword swing*
- [ ] **Espada**: golpe conectado al enemigo 📌 ("atacar y defenderse de enemigos") — *sword hit flesh*
- [ ] Plantar semilla 💡 ("plantar sus cultivos") — *plant seed in soil*
- [ ] Cosechar producto listo 💡 ("lista para ser cosechada") — *harvest crop*
- [ ] Recoger item del suelo / al inventario 💡 — *item pickup*
- [ ] Abrir / cerrar inventario 💡 — *open close inventory*
- [ ] Abrir / cerrar cofre 💡 — *open close chest*
- [ ] Colocar decoración en la granja 💡 ("se deben poder colocar en la granja desde el mismo juego") — *place furniture*

---

## 3. ❤️ Energía, vida y ciclo del día

- [ ] Alerta energía al **10 %** (empieza a caminar a media velocidad) 💡 — *low stamina warning*
- [ ] Caer **inconsciente** por energía 0 % / vida 0 % 📌 — *player collapse faint*
- [ ] Recibir golpe (baja de vida) 📌 ("cada golpe que reciba disminuirá su nivel de vida") — *player hurt*
- [ ] Alerta vida al **20 %** 💡 — *low health warning*
- [ ] **Despertar** en la cama (inicio de día) 💡 — *wake up morning*
- [ ] **2 am** / fin del día automático 💡 — *clock chime night*
- [ ] Clic en **Ok** para terminar el día (y su guardado) 💡 — *ok confirm*
- [ ] Guardado automático de la partida 💡 ("el juego automáticamente guardará la partida") — *save game*

---

## 4. 🌱 Cultivos (granja)

- [ ] Notificación de que **necesita riego** 💡 (el enunciado lo avisa con íconos; el sonido lo acompaña) — *water needed alert*
- [ ] Notificación de que está **lista la cosecha** 💡 — *crop ready chime*
- [ ] Cosecha de calidad (variantes plateado / dorado / violeta / contorno blanco) 💡 — *rare item sparkle* (opcional, una sola sirve)

---

## 5. 🐄 Animales

📌 "cuidar animales" está en la lista explícita de efectos.

- [ ] Interacción / **cuidar animal** 📌 — *pet animal happy*
- [ ] Vaca **lista para ordeñar** (notificación) 💡 — *cow moo* ← ya tenemos `Farm/Cow.mp3`, sirve
- [ ] **Ordeñar** vaca 💡 — *milk bucket*
- [ ] Gallina **pone huevo** (notificación) 💡 — *chicken cluck / egg drop*
  (el cacareo AMBIENTAL ya está conectado — ver "Ya conseguido"; la
  notificación particular de la postura queda pendiente: hoy el huevo
  aparece sin sonido propio)
- [ ] Recoger huevo 💡 — *egg pickup*
- [ ] Comprar animal (Marnie) 💡 — *purchase animal*
- [ ] Vender animal 💡 — *sell*

---

## 6. ⛏️ Caverna / mazmorra — minería

### 💡 Minerales

- [ ] Mineral **obtenido** (genérico) 💡 — *mineral obtained*
- [ ] Variante **oro** (raro, sala 5+) 💡 — *rare gold sparkle*
- [ ] **Lapislázuli se rompe** (15 % de probabilidad) 💡 — *ore break crack*
- [ ] **Plata** → aviso de que se avecina una **manada de orcos** (10 %) 💡 — *danger warning horde*
- [ ] Nivel de sala con puerta / **escalera** al siguiente 💡 — *stairs door open*

### 💡 Enemigos

- [ ] Spawn de enemigo 💡 — *monster spawn*
- [ ] Ataque **cuerpo a cuerpo** (esqueleto / orco / vampiro) 💡 — *melee attack swing*
- [ ] Enemigo **recibe daño** 💡 — *enemy hit*
- [ ] Enemigo **muere** 💡 — *enemy death*
- [ ] **Esqueleto revive** 💡 — *skeleton resurrect*
- [ ] **Vampiros: frenesí** al morir uno de la pareja 💡 — *vampire frenzy roar*
- [ ] **Calavera** lanza llama (azul y verde) 💡 — *fireball throw*
- [ ] Caer sangre / revive del vampiro 💡 — *blood splash*
- [ ] **Orcos** spawnean en grupo de 3 a 6 💡 — *orc group growl*
- [ ] Enemigo **dropea oro** al morir 💡 — *coins drop*
- [ ] Abrir **cofre abandonado** de la mazmorra 💡 — *old chest open*

### 💡 Trampas (secciones "Trampas")

- [ ] **Pinches** del suelo se activan 💡 — *floor spikes trap*
- [ ] **Lanzallamas** de pared se activa 💡 — *wall flamethrower fire*

---

## 7. 🏪 Mercado y comerciantes

- [ ] Comerciante **saluda** al cliquearlo 📌 ("que lo saludará y le dirá qué comandos reconoce") — *npc greeting*
- [ ] **Compra** efectuada 💡 — *buy transaction coins*
- [ ] **Venta** efectuada 💡 — *sell transaction coins*
- [ ] Ganar / recibir oro 💡 — *coins gain*
- [ ] **Rechazo** de pedido por el comerciante 📌 ("rechazará el pedido") — *error reject denied*
- [ ] Comando recibido (`/consultar_catalogo`, `/comprar`, `/vender`) 💡 — *command confirm*

---

## 8. 🏆 Centro comunitario

- [ ] **Donar** item a un set 💡 — *donate bundle item*
- [ ] **Set completado** (con su premio) 💡 — *bundle complete reward*
- [ ] **Juego completado**: pantalla de felicitación 📌 — *big victory fanfare* (ver música de victoria, sección 1)

---

## 9. 💬 Chat, multiplayer y regalos

- [ ] Enviar mensaje 💡 — *message send*
- [ ] Mensaje **broadcast** recibido 💡 — *message receive*
- [ ] Mensaje **privado** recibido (`@usuario`) 💡 — *private message ping*
- [ ] Jugador **se une** a la partida 💡 — *player join notification*
- [ ] **Código Multiplayer** generado 📌 (botón en settings) — *code generated chime*
- [ ] **Regalo** ofrecido 💡 — *gift offer*
- [ ] Regalo **aceptado / rechazado** 💡 — *gift accept / reject*

---

## 10. 🖥️ UI / sistema

- [ ] Clic en botón / navegación de menú 💡 — *ui button click*
- [ ] Abrir / cerrar ventanas (settings, inventario, mapa) 💡 — *window open close*
- [ ] **Mapa** abierto 📌 (sección Mapa) — *map open*
- [ ] **Retomar / unirse a partida** (código ingresado) 💡 — *join game confirm*
- [ ] **Error** / acción inválida 💡 — *error buzzer*
- [ ] **Botón de salida in-game** (cierre elegante del cliente) 💡 — *quit close app*

---

## ⚙️ Pendientes técnicos (no son audios, pero condicionan esta lista)

- [ ] 📌 **Dos canales independientes**: `settings in-game` exige **"Control de volumen de sonido"** y **"Control de volumen de música"** por separado → música y SFX no pueden compartir canal/volumen.
- [ ] 📌 Volumen **en tiempo de ejecución** (los cambios de settings se toman al instante).
- [ ] 💡 Considerar llevar los volúmenes también a un `.toml` (parametriza "valores numéricos" del juego).
- [ ] 📌 **Cliente dummy**: el cliente no decide nada — el sonido se dispara en el cliente al recibir el evento/estado del servidor (Detalle técnico #1). La "verdad" del evento es del servidor; la reproducción es presentación local.
- [ ] 📌 Sincronía **sonido ↔ animación** (Detalle técnico #3: "la música y sonido acompañando las animaciones").
- [ ] 📌 El **instalador** debe copiar los audios a `~/.local/share/NAME/` (Entregables).
- [ ] 💡 Elegir la librería de audio (el enunciado permite SDL: `SDL2_mixer` es el candidato natural) y el formato de los archivos (`.ogg` / `.mp3`).
- [ ] 💡 Transiciones de música al cambiar de zona (fade in/out) — el enunciado solo pide la música por zona, no cómo entra.
- [ ] ❌ **Fuera de alcance**: el **Editor** es un programa aparte del cliente-servidor y el enunciado no le pide audio.

---

*Última actualización: 18/10/2026 — generado a partir de `Enunciado (1).md`.*
