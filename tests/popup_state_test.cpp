#include "gtest/gtest.h"

#include "game/PopupState.h"

namespace {

using game::PopupState;
using game::PopupView;

TEST(PopupStateTest, SinDurmientesNoHayPopup) {
    PopupState popup;
    popup.showSleepState(0, 3, false, false);
    EXPECT_EQ(popup.view(), PopupView::Hidden);
}

TEST(PopupStateTest, SinVotarMuestraLaPregunta) {
    PopupState popup;
    popup.showSleepState(1, 3, false, false);
    EXPECT_EQ(popup.view(), PopupView::Question);
}

TEST(PopupStateTest, YaVotoMuestraElAvance) {
    PopupState popup;
    popup.showSleepState(1, 3, true, false);
    EXPECT_EQ(popup.view(), PopupView::Waiting);
    EXPECT_EQ(popup.waitingReady(), 1);
    EXPECT_EQ(popup.waitingTotal(), 3);
}

TEST(PopupStateTest, MuertoVeElAvanceSinPregunta) {
    // El server lo da por aceptado al morir (vida o energía): no se le
    // pregunta y espera el avance como quien ya votó.
    PopupState popup;
    popup.showSleepState(2, 4, false, true);
    EXPECT_EQ(popup.view(), PopupView::Waiting);
    EXPECT_EQ(popup.waitingReady(), 2);
    EXPECT_EQ(popup.waitingTotal(), 4);
}

TEST(PopupStateTest, ElMuertoNoTieneCancelacion) {
    // El flag lo apaga PlaySession con la vida del jugador local.
    PopupState popup;
    EXPECT_TRUE(popup.waitingCancelable());
    popup.setWaitingCancelable(false);
    EXPECT_FALSE(popup.waitingCancelable());
    popup.setWaitingCancelable(true);
    EXPECT_TRUE(popup.waitingCancelable());
}

}  // namespace
