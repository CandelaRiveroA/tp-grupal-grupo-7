#include "gtest/gtest.h"

#include "game/LocalWorldView.h"
#include "game/ObjectPicker.h"
#include "game/ViewMotion.h"
#include "world/CharacterCatalogLoader.h"
#include "world/ContentCatalog.h"
#include "world/ResourceCatalog.h"

namespace {

using game::LocalWorldView;
using game::ObjectPicker;
using game::PickTarget;
using game::ViewEntity;
using game::ViewMotion;

const world::CharacterCatalog& worldCatalog() {
    static const world::CharacterCatalog instance =
            world::CharacterCatalogLoader().loadFromFile(
                    TALLER_CHARACTERS_TOML);
    return instance;
}

// Contenido mínimo: un huevo de 16x16 dibujado.
world::ContentCatalog makeContent() {
    world::ContentCatalog content;
    world::SpriteVisual egg;
    egg.sheetImage = "Farm/egg.png";
    egg.cellWidth = 16;
    egg.cellHeight = 16;
    egg.drawnWidth = 16;
    egg.drawnHeight = 16;
    content.visuals.emplace("egg", egg);
    return content;
}

ViewEntity entityAt(int id, int cellX, int cellY) {
    ViewEntity entity;
    entity.kind = world::CreatureKind::Player;
    entity.id = id;
    entity.cellX = cellX;
    entity.cellY = cellY;
    entity.facing = world::Direction::Front;
    entity.moving = false;
    return entity;
}

TEST(ObjectPickerTest, PicksTheEntityUnderThePoint) {
    LocalWorldView view;
    view.entities.push_back(entityAt(0, 0, 0));

    ViewMotion motion(worldCatalog());
    motion.observe(view, 0);
    const world::ContentCatalog content = makeContent();
    const ObjectPicker picker(content);

    // El centro de la celda (0,0) con celdas de 32 px está en (16,16).
    const PickTarget hit =
            picker.pick(world::PixelPoint{16, 16}, view, motion, 0, 32);
    EXPECT_EQ(hit.kind, PickTarget::Kind::Entity);
    EXPECT_EQ(hit.index, 0u);

    // Muy lejos no hay nada.
    const PickTarget miss =
            picker.pick(world::PixelPoint{200, 200}, view, motion, 0, 32);
    EXPECT_EQ(miss.kind, PickTarget::Kind::None);
}

TEST(ObjectPickerTest, OverlappingEntitiesPickTheTopmost) {
    LocalWorldView view;
    view.entities.push_back(entityAt(1, 2, 2));
    // La segunda se dibuja encima: es la que tiene que recibir el pick.
    view.entities.push_back(entityAt(2, 2, 2));

    ViewMotion motion(worldCatalog());
    motion.observe(view, 0);
    const world::ContentCatalog content = makeContent();
    const ObjectPicker picker(content);

    const PickTarget hit = picker.pick(
            world::PixelPoint{2 * 32 + 16, 2 * 32 + 16}, view, motion, 0, 32);
    EXPECT_EQ(hit.kind, PickTarget::Kind::Entity);
    EXPECT_EQ(hit.index, 1u);
}

TEST(ObjectPickerTest, MovingEntityPicksAlongItsDrawnBox) {
    LocalWorldView view;
    view.entities.push_back(entityAt(7, 0, 0));

    ViewMotion motion(worldCatalog());
    motion.observe(view, 0);
    const world::ContentCatalog content = makeContent();
    const ObjectPicker picker(content);

    // Paso de (0,0) a (1,0) arrancando en el tick 0.
    view.entities[0].cellX = 1;
    view.entities[0].moving = true;
    motion.observe(view, 0);

    // A mitad de tránsito el sprite va entre ambas celdas: el pick lo
    // sigue donde está DIBUJADO, no donde está la celda destino.
    const int mid =
            worldCatalog().dataFor(world::CreatureKind::Player).cadence.stepTicks / 2;
    const PickTarget hit =
            picker.pick(world::PixelPoint{16, 16}, view, motion, mid, 32);
    EXPECT_EQ(hit.kind, PickTarget::Kind::Entity);
    EXPECT_EQ(hit.index, 0u);
}

TEST(ObjectPickerTest, PicksTheDroppedItemUnderThePoint) {
    LocalWorldView view;
    view.droppedItems.push_back(game::ViewDroppedItem{"egg", 2, 2});

    ViewMotion motion(worldCatalog());
    motion.observe(view, 0);
    const world::ContentCatalog content = makeContent();
    const ObjectPicker picker(content);

    // Huevo de 16x16 en la celda (2,2): su caja dibujada es x=72..88,
    // y=80..96 (centrada y apoyada abajo de la celda de 32 px).
    const PickTarget hit =
            picker.pick(world::PixelPoint{80, 90}, view, motion, 0, 32);
    EXPECT_EQ(hit.kind, PickTarget::Kind::DroppedItem);
    EXPECT_EQ(hit.index, 0u);

    // Dentro de la celda pero FUERA de la caja del sprite: no hay hit.
    // Lo que se toca es lo que se ve.
    const PickTarget miss =
            picker.pick(world::PixelPoint{64, 64}, view, motion, 0, 32);
    EXPECT_EQ(miss.kind, PickTarget::Kind::None);
}

TEST(ObjectPickerTest, EntitiesWinOverItems) {
    LocalWorldView view;
    view.entities.push_back(entityAt(1, 2, 2));
    view.droppedItems.push_back(game::ViewDroppedItem{"egg", 2, 2});

    ViewMotion motion(worldCatalog());
    motion.observe(view, 0);
    const world::ContentCatalog content = makeContent();
    const ObjectPicker picker(content);

    // Ambos ocupan la misma celda: la entidad se dibuja encima del ítem,
    // así que es la que recibe el pick.
    const PickTarget hit = picker.pick(
            world::PixelPoint{2 * 32 + 16, 2 * 32 + 16}, view, motion, 0, 32);
    EXPECT_EQ(hit.kind, PickTarget::Kind::Entity);
    EXPECT_EQ(hit.index, 0u);
}

}  // namespace
