#include <gtest/gtest.h>

#include "game/ObjectPicker.h"
#include "game/SceneTransition.h"
#include "game/SceneSnapshot.h"

TEST(SleepRegressionTest, CamaDelServidorSePuedePulsarEnTodaSuCaja) {
    world::ContentCatalog content;
    world::SpriteVisual visual;
    visual.cols = 2;
    visual.rows = 4;
    content.visuals.emplace("bed", visual);
    const game::ObjectPicker picker(content);
    const world::CharacterCatalog characters;
    const game::ViewMotion motion(characters);
    game::LocalWorldView view;
    view.placedFurniture.push_back(game::ViewFurniture{"bed", 9, 5});

    // Base (9,5), esquina superior (9,2), celdas de 32 px.
    for (const world::PixelPoint point: {world::PixelPoint{288, 64},
                                        world::PixelPoint{351, 191}}) {
        const game::PickTarget target = picker.pick(point, view, motion, 0, 32);
        EXPECT_EQ(target.kind, game::PickTarget::Kind::Furniture);
        EXPECT_EQ(target.index, 0u);
    }
    EXPECT_EQ(picker.pick({352, 192}, view, motion, 0, 32).kind,
              game::PickTarget::Kind::None);
}

TEST(SleepRegressionTest, MuebleSinVisualNoCapturaClicks) {
    const world::ContentCatalog content;
    const game::ObjectPicker picker(content);
    const world::CharacterCatalog characters;
    const game::ViewMotion motion(characters);
    game::LocalWorldView view;
    view.placedFurniture.push_back(game::ViewFurniture{"unknown", 9, 5});
    EXPECT_EQ(picker.pick({288, 160}, view, motion, 0, 32).kind,
              game::PickTarget::Kind::None);
}

TEST(SleepRegressionTest, FinDeDiaEsperaEnNegroHastaDespertar) {
    game::SceneTransition transition(2);
    transition.changeTo("cabin_1", true);
    transition.tick(0);
    transition.tick(2);
    EXPECT_TRUE(transition.holding());
    EXPECT_FALSE(transition.swapDue());
    transition.tick(100);
    EXPECT_EQ(transition.alpha(), 255);
    transition.release();
    EXPECT_TRUE(transition.swapDue());
    transition.consumeSwap();
    transition.tick(101);
    transition.tick(103);
    EXPECT_FALSE(transition.active());
}

TEST(SleepRegressionTest, FinDeDiaDuranteSalidaDeMapaConservaLaEspera) {
    game::SceneTransition transition(2);
    transition.changeTo("cabin_1");
    transition.tick(0);
    transition.changeTo("farm", true);
    transition.tick(2);
    EXPECT_TRUE(transition.holding());
    EXPECT_EQ(transition.scene(), "cabin_1");
}

TEST(SleepRegressionTest, FinDeDiaDuranteEntradaDeMapaConservaLaEspera) {
    game::SceneTransition transition(2);
    transition.changeTo("cabin_1");
    transition.tick(0);
    transition.tick(2);
    transition.consumeSwap();
    transition.changeTo("farm", true);
    transition.tick(3);
    transition.tick(5);
    EXPECT_TRUE(transition.holding());
    EXPECT_EQ(transition.scene(), "cabin_1");
}

TEST(SleepRegressionTest, DatosDeCabanaNoSeDibujanSobreLaGranja) {
    game::SceneSnapshot snapshot;
    game::LocalWorldView farm;
    farm.mapId = "farm";
    farm.entities.push_back(game::ViewEntity{});
    farm.entities[0].cellX = 20;
    snapshot.observe(farm, "farm");

    game::LocalWorldView cabin;
    cabin.mapId = "cabin_2";
    cabin.entities.push_back(game::ViewEntity{});
    cabin.entities[0].cellX = 9;
    cabin.placedFurniture.push_back(game::ViewFurniture{"bed", 9, 5});
    const game::LocalWorldView& fading = snapshot.observe(cabin, "farm");
    EXPECT_EQ(fading.mapId, "farm");
    EXPECT_TRUE(fading.placedFurniture.empty());
    EXPECT_EQ(fading.entities[0].cellX, 20);

    const game::LocalWorldView& arrived = snapshot.observe(cabin, "cabin_2");
    EXPECT_EQ(arrived.mapId, "cabin_2");
    EXPECT_EQ(arrived.placedFurniture.size(), 1u);
    EXPECT_EQ(arrived.entities[0].cellX, 9);
}

TEST(SleepRegressionTest, DespertarActualizaDestinoAntesDeSoltarElNegro) {
    game::SceneTransition transition(2);
    transition.changeTo("farm", true);
    transition.tick(0);
    transition.tick(2);
    EXPECT_TRUE(transition.holding());
    transition.changeTo("cabin_2");
    transition.release();
    EXPECT_TRUE(transition.swapDue());
    EXPECT_EQ(transition.scene(), "cabin_2");
}

TEST(SleepRegressionTest, TrasladoAntesDeNegroActualizaElDestino) {
    game::SceneTransition transition(2);
    transition.changeTo("farm", true);
    transition.tick(0);
    transition.changeTo("cabin_3");
    transition.tick(2);
    EXPECT_TRUE(transition.holding());
    EXPECT_EQ(transition.scene(), "cabin_3");
}
