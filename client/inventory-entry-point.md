# Punto de entrada: inventario

Documento para quien conecta **cliente ↔ servidor**. Explica **dónde** entra la
decisión del servidor sobre las herramientas del inventario, y **cómo** se le
asigna la posición exacta en pantalla. Es el equivalente, para el inventario,
de lo que `connection-point.md` describe para `GameClient` y las dimensiones.

El cliente es **dummy**: el servidor decide **qué** herramientas van y en **qué
slot**; el cliente solo **dibuja**. El cliente no decide nada de juego, solo
presentación (posiciones en píxeles y sprites).

## TL;DR
- El servidor manda **qué** herramientas y en **qué slot** (0..8).
- Eso llega como `game::LocalWorldView.inventory` → `vector<{slotIndex, item}>`.
- El cliente decide **dónde** se dibuja cada slot: `inventory.toml` +
  `InventoryLayout` (ancla abajo-centro, `count` slots).
- El **punto de entrada** (donde se lee y se asigna) es `sdl::InventoryHud::draw`.

## El flujo, paso a paso
```
servidor  (hoy: MockGameClient simula)  decide "qué herramientas en qué slot"
   └─► view_.inventory = [{slotIndex, item}, ...]      LocalWorldView.inventory
         └─► GameClient::worldState()                  RECIBIR (ver connection-point.md)
               └─► InventoryHud::draw(inventory)  ◄──── PUNTO DE ENTRADA (lee + asigna)
                      ├─ boxes = layout.slotBoxes(w,h)  posiciones de los 9 slots (CLIENTE)
                      └─ por cada entrada del servidor:
                            slotIndex ► boxes[slotIndex] asigna POSICIÓN en pantalla
                            item      ► catálogo visual  asigna APARIENCIA (hoja + celda)
```

## Qué manda el servidor — contenido
Definición compartida: `client/game/LocalWorldView.h`
```cpp
struct ViewInventorySlot {
    int             slotIndex;   // slot LÓGICO, 0..8
    world::ItemKind item;        // qué herramienta
};
```
`ItemKind` (`client/world/ItemCatalog.h`): `Shovel`, `Pickaxe`, `WateringCan`,
`Sword`.

- El servidor manda **solo** slot lógico + item.
- **Nunca** manda píxeles, hojas ni sprites: eso es presentación del cliente.
- Máximo **9 slots** (límite del enunciado). Un `slotIndex >= 9` se ignora.

## Qué decide el cliente — posición
Datos (se editan sin recompilar): `client/sdl/assets/inventory.toml`
- `[bar] count = 9` → cuántos slots.
- `scale`, `gap`, `anchor = "bottom_center"`, `margin`, `ui_scale` → dónde se
  ancla la barra y qué tan grande es.
- `[items.<nombre>]` → mapa de item lógico → hoja + celda del sprite.

Cálculo de la posición: `client/world/InventoryLayout.cpp`
- `outerBox()` → ancla la barra abajo y centrada.
- `slotBoxes()` → reparte los `count` slots en píxeles.
- `iconBox()` → deja el hueco del ítem dentro de un slot.

Detalle de cada campo del toml: `client/world/inventory-format.md`.

## Dónde se lee y se asigna (el punto de entrada)
`client/sdl/render/InventoryHud.cpp` →
`InventoryHud::draw(const std::vector<game::ViewInventorySlot>&)`:
```cpp
const std::vector<world::PixelBox> boxes =
    layout_.slotBoxes(context_.width(), context_.height());

for (const game::ViewInventorySlot& entry : inventory) {
    if (entry.slotIndex < 0 || entry.slotIndex >= slots) continue; // fuera de rango → ignora
    if (!layout_.items.has(entry.item)) continue;                  // sin visual → ignora
    const world::ItemVisual& visual = layout_.items.visualFor(entry.item);
    const world::PixelBox icon =
        layout_.iconBox(boxes[static_cast<std::size_t>(entry.slotIndex)]); // ASIGNA POSICIÓN
    itemSheet.draw(visual.col, visual.row, icon);                          // ASIGNA APARIENCIA
}
```
- `entry.slotIndex` (del servidor) elige **dónde**: el box que el cliente ya
  calculó con su layout.
- `entry.item` (del servidor) elige **qué sprite**, vía el catálogo `[items.*]`.

## Integrar la implementación de red
1. Implementar una clase que herede de `game::GameClient` (`sendCommand` +
   `worldState`), como describe `connection-point.md`.
2. En `worldState()`, llenar `inventory` con un `ViewInventorySlot` por
   herramienta que mande el servidor.
3. Nada más: `InventoryHud::draw` ya la dibuja. **El cliente SDL no cambia.**
4. Referencia de comportamiento: `MockGameClient::rebuildInventory()`
   (`client/game/MockGameClient.cpp`) es el "servidor falso" que hoy llena ese
   mismo vector con pala/pico/espada/regadera.

## Reglas / edge cases
- `slotIndex` fuera de `[0, count)` → la entrada se ignora (no rompe).
- `ItemKind` sin entrada en `[items.*]` del toml → el ítem se ignora (sin error).
- Dos entradas con el mismo `slotIndex` → se dibujan superpuestas (hoy no se
  deduplica).
