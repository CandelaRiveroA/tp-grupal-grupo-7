# Cheats y comandos de chat

Los comandos de esta guía están registrados en `server/commands/chat_command.cpp`.
Se ejecutan desde el chat y sus efectos los aplica el servidor.

## Convenciones

- Escribir los comandos en minúsculas y separar palabras con `_`.
- Los parámetros entre `[corchetes]` son obligatorios; no escribir los corchetes.
- Las cantidades son números enteros. Solo `/oro` interpreta los negativos como una resta.
- Los comandos sin parámetros rechazan argumentos adicionales.
- Las respuestas de los comandos se envían únicamente a quien los ejecuta.

## Ayuda

| Comando | Descripción | Ejemplo |
| --- | --- | --- |
| `/info` | Muestra todos los comandos registrados, incluidos los de comerciantes. La lista se genera a partir del registro del servidor. | `/info` |

## Vida y energía

| Comando | Descripción | Ejemplo |
| --- | --- | --- |
| `/vida [cantidad]` | Fija la vida del jugador. Limita el valor entre 0 y su vida máxima actual; con 0 dispara la muerte y el traslado a la cama mediante el flujo habitual. | `/vida 10` |
| `/energia [cantidad]` | Fija la energía en puntos enteros de la escala visible (normalmente 0 a 100). Limita el valor a la energía máxima actual; con 0 dispara el desmayo. | `/energia 10` |
| `/invencibilidad [estado]` | Activa (`1`) o desactiva (`0`) la protección individual contra la pérdida de vida y energía. | `/invencibilidad 1` |

La invencibilidad empieza desactivada. Al activarla no cura ni revive, pero
bloquea el daño, el consumo de energía y las reducciones de ambos valores
(incluidas las de `/vida`, `/energia` y las penalizaciones al despertar).
Permite recuperarlos y conserva su estado al cambiar de mapa o de día.
El reloj y el fin de día mantienen su funcionamiento normal.

## Objetos

| Comando | Descripción | Ejemplo |
| --- | --- | --- |
| `/dar [objeto] [calidad] {cantidad}` | Agrega objetos al inventario propio. La cantidad es opcional y por defecto vale 1; admite de 1 a 65.535. Si no entra todo, conserva lo que entró, informa la cantidad y envía el aviso de inventario lleno. | `/dar frutilla perfecta 3`, `/dar carbon base 10`, `/dar cofre 1` |
| `/dar_info` | Lista todos los nombres de objetos aceptados por `/dar` desde el mismo catálogo que usa para crearlos. También muestra las calidades válidas. | `/dar_info` |

Calidades admitidas: `1`/`base`, `2`/`buena`, `3`/`excelente`,
`4`/`increible`, `5`/`perfecta`. Se escriben sin tildes.
La calidad elegida se aplica a productos, semillas y recursos. Las herramientas,
la cama, el cofre y las decoraciones solo admiten `1`/`base`.

Los nombres coinciden con los comandos en español: por ejemplo `pico`, `pala`,
`regadera`, `espada`, `frutilla`, `semilla_frutilla`, `leche`, `huevo`, `carbon`,
`lapislazuli`, `plata`, `oro`, `roca`, `roca_2` a `roca_6`, `cama`, `cofre`, `decoracion_1` y `decoracion_2`.
La lista completa y vigente se consulta con `/dar_info`.
`/dar oro base` entrega el recurso mineral; `/oro` modifica las monedas.
Los animales son entidades del granero y se compran a Marnie, no se entregan
como objetos de inventario mediante `/dar`.

## Oro

| Comando | Descripción | Ejemplo |
| --- | --- | --- |
| `/oro [cantidad]` | Agrega o quita oro. El saldo queda limitado entre 0 y 65.535. | `/oro 5000`, `/oro -200` |

## Centro comunitario

| Comando | Descripción | Ejemplo |
| --- | --- | --- |
| `/completar_set [nombre_set]` | Completa un lote y aplica su recompensa una sola vez. Nombres válidos: `cultivos`, `minerales`, `animales`, `pelea` y `dinero`. Para minerales también acepta `metales` y `orbes`. | `/completar_set metales` |
| `/completar_todo` | Completa todos los lotes pendientes, aplica sus recompensas y dispara la victoria al completar el centro. | `/completar_todo` |

`/donar` fue eliminado. Las donaciones se realizan con el botón **Donar** del
popup del lote. El servidor valida el mapa y el radio de una celda alrededor
del edificio, consume solo lo pendiente y admite aportes parciales. El lote de
dinero recibe hasta 1.000 de oro por clic, limitado al saldo disponible y a lo
que falte. El progreso es compartido por todos los jugadores.

## Escenarios y mazmorra

| Comando | Descripción | Ejemplo |
| --- | --- | --- |
| `/tp [escenario]` | Teletransporta al escenario indicado y busca una celda caminable. También permite entrar en un piso de la mazmorra usando su ID. | `/tp town`, `/tp dungeon_5` |
| `/tp_info` | Lista los IDs de los escenarios disponibles en la partida. | `/tp_info` |

## Granja y granero

| Comando | Descripción | Ejemplo |
| --- | --- | --- |
| `/cuidar_animales` | Prepara la producción de todos los animales del granero: cada gallina pone un huevo en su celda y cada vaca queda lista para ordeñar. | `/cuidar_animales` |
| `/cuidar_cultivos` | Lleva todos los cultivos plantados de la granja a su etapa cosechable, incluidos los rebrotes. | `/cuidar_cultivos` |

Ambos comandos funcionan desde cualquier mapa y **no avanzan el día**.

- La leche se obtiene ordeñando y los huevos se recogen del piso con las
  interacciones habituales; el cheat no los agrega al inventario.
- Cada ejecución de `/cuidar_animales` genera un nuevo huevo por gallina. Las
  vacas que ya tenían leche disponible siguen listas, sin acumular varias leches.
- `/cuidar_cultivos` no cosecha ni simula días adicionales de riego. Conserva el
  cálculo de calidad según el riego acumulado y la suerte. Los cultivos que ya
  estaban maduros conservan su calidad al repetir el comando.

## Comerciantes

Estos comandos utilizan el comerciante seleccionado o uno cercano, según las
validaciones del servidor. Los productos disponibles dependen de su catálogo.

| Comando | Descripción | Ejemplo |
| --- | --- | --- |
| `/consultar_catalogo` | Consulta el catálogo del comerciante y envía sus ofertas al cliente. | `/consultar_catalogo` |
| `/comprar [producto] [cantidad]` | Compra una cantidad positiva del producto indicado, sujeta a saldo, espacio y disponibilidad. | `/comprar vacas 1` |
| `/vender [producto] [cantidad]` | Vende una cantidad positiva de un producto aceptado por el comerciante. Marnie acepta huevos, leche y animales de la granja. | `/vender huevo 2` |

## Tiempo

| Comando | Descripción | Ejemplo |
| --- | --- | --- |
| `/terminar_dia` | Fuerza el fin del día y ejecuta el flujo habitual de desmayo, espera y comienzo del siguiente día. | `/terminar_dia` |
| `/tiempo [minutos]` | Fija el reloj entre 0 y 1.200 minutos. El reloj continúa desde ese valor; llegar a 1.200 dispara el fin del día. | `/tiempo 600`, `/tiempo 1200` |

## Alcance en multijugador

- Vida, energía, oro, objetos, invencibilidad y teletransporte afectan al jugador que ejecuta el comando.
- Los lotes comunitarios, animales, cultivos y el reloj pertenecen a la partida:
  sus cambios afectan al estado compartido.
- Los cheats de cuidado reutilizan las listas de animales y cultivos del
  dominio. Los clientes reciben los cambios mediante la sincronización existente.
