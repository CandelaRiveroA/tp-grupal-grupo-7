# Cheats

```
/comando [parametro_1] [parametro_2] ...
```

## Convenciones

- Los comandos y parámetros se escriben en **español**, en minúsculas y usando
  guión bajo `_` para separar palabras (`cuidar_cultivos`).
- Los parámetros entre `[corchetes]` son **obligatorios**.
- Los parámetros entre `{llaves}` son **opcionales**; si se omiten, se aplica el
  valor por defecto indicado en la descripción.
- Las cantidades (`X`) son números enteros. Donde tiene sentido, se aceptan
  valores negativos para quitar en lugar de agregar.
- Si el comando o sus parámetros son inválidos, el servidor responde con un
  mensaje explicativo por el chat, sin aplicar ningún efecto.

## Tabla de cheats

### Vida y energía

| Comando | Descripción | Ejemplo |
| --- | --- | --- |
| `/vida [cantidad]` | Fija la vida actual del jugador en el valor indicado. Permite probar las condiciones del 20% (mitad de efectividad), del 0% (caer inconsciente) y niveles muy altos de vida. | `/vida 10` |
| `/energia [cantidad]` | Fija la energía actual del jugador en el valor indicado. Permite probar las condiciones del 10% (media velocidad), del 0% (caer inconsciente) y niveles muy altos de energía. | `/energia 100` |
| `/curar` | Restaura la vida y la energía del jugador al máximo, como al despertar en la cama. | `/curar` |

### Oro

| Comando | Descripción | Ejemplo |
| --- | --- | --- |
| `/oro [cantidad]` | Agrega la cantidad de monedas de oro indicada a la billetera del jugador. Si la cantidad es negativa, se quita oro (sin bajar de 0). | `/oro 5000` |
| `/oro [cantidad]` | Quitar oro usando un valor negativo. | `/oro -200` |

### Centro comunitario

| Comando | Descripción | Ejemplo |
| --- | --- | --- |
| `/completar_set [nombre_set]` | Completa un set (bundle) específico del centro comunitario y otorga su premio. Valores válidos: `cultivos`, `metales`, `animales`, `pelea`, `dinero`. | `/completar_set metales` |
| `/completar_todo` | Completa todos los sets del centro comunitario de una sola vez. Útil para probar la pantalla de victoria del juego. | `/completar_todo` |
| `/donar [set] [item] [cantidad]` | Dona productos o recursos de la mochila al set indicado. | `/donar cultivos frutilla 1` |
| `/donar dinero [cantidad]` | Dona oro de la billetera al set de dinero. El set se completa al acumular 10.000 oro. | `/donar dinero 10000` |

### Mazmorra

| Comando | Descripción | Ejemplo |
| --- | --- | --- |
| `/siguiente_nivel` | Avanza automáticamente al siguiente nivel de la mazmorra, sin tener que buscar la puerta o escalera. | `/siguiente_nivel` |
| `/ir_nivel [numero]` | Teletransporta al jugador directamente al nivel indicado de la mazmorra (1 a 10). Útil para probar, por ejemplo, la aparición de oro a partir de la 5ta sala. | `/ir_nivel 5` |

### Granja

| Comando | Descripción | Ejemplo |
| --- | --- | --- |
| `/cuidar_cultivos` | Riega y atiende automáticamente todos los cultivos de la granja que lo necesiten. | `/cuidar_cultivos` |
| `/cuidar_animales` | Atiende automáticamente a todos los animales de la granja (ordeñar vacas, recoger huevos, etc.). | `/cuidar_animales` |

### Objetos (utilidad para pruebas)

| Comando | Descripción | Ejemplo |
| --- | --- | --- |
| `/dar [objeto] {cantidad}` | Agrega al inventario del jugador la cantidad indicada del objeto (por defecto 1). Útil para completar sets o probar el mercado sin minar/cultivar. | `/dar orbe_oro 3` |

### Tiempo

| Comando | Descripción | Ejemplo |
| --- | --- | --- |
| `/terminar_dia` | Fuerza el fin del día actual, como si el reloj llegara a las 2 am. Dispara el guardado automático de la partida. | `/terminar_dia` |

## Notas de diseño

- En partidas multiplayer, los cheats que afectan al centro comunitario
  (`/completar_set`, `/completar_todo`) impactan a todos los jugadores, ya que
  los sets son compartidos. Los cheats de vida, energía, oro e inventario sólo
  afectan al jugador que los ejecuta.
- Como todo pasa por el servidor, conviene centralizar el parseo de estos
  comandos junto al de los comandos de comerciantes, reutilizando el mismo
  mecanismo de `/comando parametros`.
