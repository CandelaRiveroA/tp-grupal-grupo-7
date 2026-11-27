# TODO — Limpieza pendiente del editor de mundos

Cosas temporales o de fallback en el código que se pueden eliminar una vez que se definan los assets o fases faltantes.

---

## 1. Cuando se defina el prefab del Mercado (`role = "market"`)

Hoy `casa_granja_1` (`cabin`) y `centro_comunitario` (`community_center`) ya tienen prefab en [`editor/prefabs.toml`](../../editor/prefabs.toml), pero **no hay prefab para `market`**, por lo que sigue usando el recuadro placeholder.

Una vez agregado `role = "market"` y su `door` en [`editor/tools/build_prefabs.py`](../../editor/tools/build_prefabs.py) (y regenerado `prefabs.toml`):

- [ ] **Eliminar el fallback de [`WorldFactory::placeBuilding`](../../common/world_model/WorldFactory.cpp#L233-L256)** y `WorldFactory::defaultObjectTile`: si `catalog.findByRole(role) == nullptr`, lanzar `FormatError`.
- [ ] **Eliminar las constantes `kFallbackCabin*`, `kFallbackMarket*` y `kFallbackCenter*`** de [`common/world_model/Constants.h`](../../common/world_model/Constants.h).
- [ ] **Quitar el parámetro por defecto `= {}`** de `catalog` en [`WorldFactory::replaceMapScenario`](../../common/world_model/WorldFactory.h#L45) y pasar `defaultCatalog()` en los tests de error de [`tests/world_factory_test.cpp`](../../tests/world_factory_test.cpp).
- [ ] **Eliminar el recuadro amarillo placeholder** en [`editor/MapLayerItem.cpp`](../../editor/MapLayerItem.cpp) (`drawObjects`, rama `!obj.tile.tileset.starts_with("prefab_")`).

---

## 2. Cuando estén los escenarios reales de interiores

- [ ] Incorporar los escenarios reales de `cabin` y `community_center` en `editor/scenarios/`.
- [ ] Verificar que las coordenadas por defecto de `Bed` e `InteriorExit` en [`WorldFactory::placeMapPoints`](../../common/world_model/WorldFactory.cpp#L258-L290) caigan sobre celdas caminables de esos escenarios.

---

## 3. Pendiente de Fase 4 (Plan 2) y Fases 5/6

- [ ] **Plan 2 de Fase 4** ([`docs/roadmap-editor-mundos-fase-4.md`](../roadmap-editor-mundos-fase-4.md)): alta/baja de cabañas en `farm` sincronizando `cabin_1..cabin_N`, pestaña **Elementos**, reubicación, portales `link` y *"Elegir destino"*.
- [ ] **Fases 5 y 6:** derivar el cupo máximo de jugadores (`1..4`) contando las cabañas (`ObjectRole::Cabin`) en `farm` (o los mapas `cabin_k`), y asignar una cabaña libre con su `bed` a cada jugador que entra.
