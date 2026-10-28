# Diferencia entre SDL y Qt

## Pantallas de inicio (Qt)

Pantallas iniciales gestionadas por Qt:

- Crear partida.
- Unirse a una partida existente (doble clic en la partida → "Unirme").

Una vez que Qt termina su parte, el resto del juego lo maneja SDL.

## Transición de Qt a SDL

- Qt se encarga de configurar el personaje (ropa, apariencia, etc.), tal cual como el jugador lo eligió.
- Al empezar la partida, el personaje aparece en SDL **ya configurado** con esos cambios.
- SDL se encarga de todo lo demás que sucede dentro del juego.

## Delegación entre SDL y Qt

- El `main` del cliente es de Qt.
- Qt habla con el protocolo para validar lo que decidió el jugador (ej.: partida inexistente, usuario tomado, etc.).
- Una vez confirmado todo, se llama lo que llamamos "main SDL": el inicio de la partida.
- La configuración de volumen y demás opciones **dentro de la partida** también están en SDL.
- El multiplayer también está en SDL.

## Temas generales

### Sonidos

- Usar extensiones de audio con la menor memoria posible para no sobrecargar la computadora.

### Assets (sprites)

- Cargar la imagen en memoria y renderizar únicamente un cuadradito (frame) de esa imagen.
- Usar tablas/tiles grandes: la idea es entender la equidistancia entre frames y moverse de frame en frame para animar.
- Hay que sincronizar (timear) el desplazamiento con el movimiento en X e Y.

### Bucle de tasa constante ("constant rate loop")

- Pendiente: buscar el artículo para implementarlo después.
- Permite mantener la visualización del juego actualizada a tiempo (render a ritmo constante).

## Flujo de movimiento en SDL

1. El jugador toca una tecla.
2. El lado de SDL detecta la tecla y se la envía al servidor.
3. El servidor responde si el movimiento está permitido ("¿me puedo mover?").
4. El status devuelvo decide si el movimiento se realiza o no (ej.: no, porque hay una pared).

## Features

### Minería (lógica a nivel SDL)

- El personaje pica una piedra y caen las piedritas.
- Inventario: poder recoger el mineral del suelo.
- Debe existir el sprite para picar.

### Granja

- Lógica para ver vacas y gallinas, y al menos tener lo visual.

## Importante

- **El cliente es dummy**: toda la lógica de juego vive en el servidor; el cliente solo renderiza y envía inputs.
