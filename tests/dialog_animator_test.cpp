#include "game/DialogAnimator.h"

#include "game/LocalWorldView.h"
#include "gtest/gtest.h"

namespace {

using game::DialogAnimator;
using game::LocalWorldView;
using game::ViewEntity;

ViewEntity entityWith(int id, bool ready) {
    ViewEntity entity;
    entity.kind = world::CreatureKind::Player;
    entity.id = id;
    entity.cellX = 0;
    entity.cellY = 0;
    entity.facing = world::Direction::Front;
    entity.moving = false;
    if (ready) {
        entity.indication = world::Indication::ReadyToMilk;
    }
    return entity;
}

TEST(DialogAnimatorTest, AnimationStartsWhenTheIndicationAppears) {
    LocalWorldView view;
    view.entities.push_back(entityWith(7, true));

    DialogAnimator animator;
    animator.observe(view, 10);

    EXPECT_EQ(animator.animationTickFor(view.entities[0], 10), 0);
    EXPECT_EQ(animator.animationTickFor(view.entities[0], 15), 5);
}

TEST(DialogAnimatorTest, RestartAfterTheIndicationDisappears) {
    LocalWorldView view;
    view.entities.push_back(entityWith(7, true));

    DialogAnimator animator;
    animator.observe(view, 10);

    // Desaparece la indicación y vuelve después: la animación arranca
    // de cero (one-shot de cada aparición).
    view.entities[0].indication.reset();
    animator.observe(view, 20);
    view.entities[0].indication = world::Indication::ReadyToMilk;
    animator.observe(view, 50);

    EXPECT_EQ(animator.animationTickFor(view.entities[0], 50), 0);
    EXPECT_EQ(animator.animationTickFor(view.entities[0], 53), 3);
}

TEST(DialogAnimatorTest, EntitiesWithoutIndicationHaveNoTicks) {
    LocalWorldView view;
    view.entities.push_back(entityWith(7, false));

    DialogAnimator animator;
    animator.observe(view, 10);

    EXPECT_EQ(animator.animationTickFor(view.entities[0], 15), 0);
}

TEST(DialogAnimatorTest, CropsAnimateTheirDialogLikeEntities) {
    LocalWorldView view;
    game::ViewCrop thirsty{3, 4, "strawberry", 0};
    thirsty.indication = world::Indication::ReadyToWater;
    game::ViewCrop quiet{5, 4, "strawberry", 2};
    view.plantedCrops.push_back(thirsty);
    view.plantedCrops.push_back(quiet);

    DialogAnimator animator;
    animator.observe(view, 10);

    EXPECT_EQ(animator.animationTickFor(view.plantedCrops[0], 10), 0);
    EXPECT_EQ(animator.animationTickFor(view.plantedCrops[0], 14), 4);
    // Sin indicación no hay animación.
    EXPECT_EQ(animator.animationTickFor(view.plantedCrops[1], 14), 0);
}

}  // namespace
