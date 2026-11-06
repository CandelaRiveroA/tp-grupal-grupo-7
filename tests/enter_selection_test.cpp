#include <optional>

#include "game/EnterSelection.h"
#include "game/LocalWorldView.h"
#include "gtest/gtest.h"

#include "scene_fixtures.h"

namespace {

using game::EnteredFront;
using game::EnterSelection;
using game::LocalWorldView;
using game::ViewEntity;

constexpr int kCellPixels = 16;

// Escena mínima con una cama entrable: pieza de 2×2 celdas, filas
// frontales 0-1, lie en (5,4) mirando a la derecha.
const char* const kBedroomToml = R"(
[tilesets.farm]
image = "Farm/walls&floors.png"
cell  = [16, 16]

[rooms.bedroom]
floor_color = [0, 0, 0, 255]
grid        = [10, 10]
scale       = 1

  [[rooms.bedroom.elements]]
  tileset = "farm"
  region  = [0, 0, 2, 2]
  draw    = { mode = "tile" }
  at      = [4, 3]
  span    = [2, 2]
  enter   = { rows = [0, 1], lie = [5, 4], facing = "right" }
)";

ViewEntity playerAt(int cellX, int cellY) {
    return ViewEntity{
            world::CreatureKind::Player, cellX,       cellY, world::Direction::Front, true,
            world::ActionKind::Walk,     std::nullopt};
}

// El remoto que duerme (sleeping lo manda el server) se ve tapado en la
// otra pantalla: pose del elemento + capa frontal, aunque el local esté
// lejos de la cama.
TEST(EnterSelectionTest, RemotoEnLaCamaSeVeTapiado) {
    const world::SceneDescription scene = scene_fixtures::loadString(kBedroomToml, "bedroom");
    const EnterSelection selection(scene, kCellPixels);

    ViewEntity sleeper = playerAt(5, 4);
    sleeper.sleeping = true;
    LocalWorldView view;
    view.entities.push_back(playerAt(1, 1));  // local, lejos
    view.entities.push_back(sleeper);         // remoto durmiendo, en lie

    const LocalWorldView drawn = selection.displayView(view);
    ASSERT_EQ(drawn.entities.size(), 2);
    // El remoto queda en la pose del elemento...
    EXPECT_EQ(drawn.entities[1].facing, world::Direction::Right);
    EXPECT_FALSE(drawn.entities[1].moving);
    EXPECT_EQ(drawn.entities[1].action, world::ActionKind::Idle);
    // ...y el local, que está lejos, no cambia.
    EXPECT_EQ(drawn.entities[0].facing, world::Direction::Front);
    EXPECT_TRUE(drawn.entities[0].moving);
    EXPECT_EQ(drawn.entities[0].action, world::ActionKind::Walk);

    // La capa de la manta se dibuja aunque el local esté lejos.
    const std::optional<EnteredFront> front = selection.front(view);
    ASSERT_TRUE(front.has_value());
    EXPECT_EQ(front->firstRow, 0);
    EXPECT_EQ(front->lastRow, 1);
}

// Sin nadie en la celda lie no hay pose ni capa frontal.
TEST(EnterSelectionTest, SinNadieEnLaCamaNoHayPoseNiCapa) {
    const world::SceneDescription scene = scene_fixtures::loadString(kBedroomToml, "bedroom");
    const EnterSelection selection(scene, kCellPixels);

    LocalWorldView view;
    view.entities.push_back(playerAt(1, 1));

    const LocalWorldView drawn = selection.displayView(view);
    EXPECT_EQ(drawn.entities[0].action, world::ActionKind::Walk);
    EXPECT_TRUE(drawn.entities[0].moving);
    EXPECT_FALSE(selection.front(view).has_value());
}

}  // namespace
