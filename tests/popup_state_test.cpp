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
    EXPECT_EQ(popup.promptId(), world::PromptId::SleepQuestion);
}

TEST(PopupStateTest, LaPreguntaDeSuenoNoHerdaLaCopiaAnterior) {
    // Al pasar del popup de cambio de día (o de regalo) a la pregunta de
    // sueño, la copia tiene que ser la de SleepQuestion: si no, el painter
    // dibuja la copia vieja (sin opciones/botón) y el popup queda sin
    // respuesta. Regresión del bug "botón de irse a la cama que no aparece".
    PopupState popup;
    popup.showNextDay();
    EXPECT_EQ(popup.promptId(), world::PromptId::NextDay);
    popup.showSleepState(1, 3, false, false);
    EXPECT_EQ(popup.view(), PopupView::Question);
    EXPECT_EQ(popup.promptId(), world::PromptId::SleepQuestion);

    PopupState popup2;
    popup2.showGiftReceived("Teo");
    popup2.showSleepState(1, 3, false, false);
    EXPECT_EQ(popup2.view(), PopupView::Question);
    EXPECT_EQ(popup2.promptId(), world::PromptId::SleepQuestion);
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

TEST(PopupStateTest, ElPedidoDeRegaloMuestraElNombreDelEmisor) {
    // El nombre viene de la red (ServerGiftRequest.characterName) y el
    // popup lo usa como {0}. Se sobreescribe en cada pedido: no queda
    // ninguno viejo visible (hide solo oculta, el valor no se lee).
    PopupState popup;
    popup.showGiftReceived("Teo");
    EXPECT_EQ(popup.view(), PopupView::GiftReceived);
    EXPECT_EQ(popup.promptId(), world::PromptId::GiftReceived);
    EXPECT_EQ(popup.giftSenderName(), "Teo");
}

TEST(PopupStateTest, OfrecerRegaloAbreLaOfertaYLaEspera) {
    // El click sobre otro jugador abre GiftOffer; al responder Sí el
    // cliente pasa a GiftWaiting (mismo popup modal, otra copia).
    PopupState popup;
    popup.showGiftOffer();
    EXPECT_EQ(popup.view(), PopupView::GiftOffer);
    EXPECT_TRUE(popup.modal());
    popup.showGiftWaiting();
    EXPECT_EQ(popup.view(), PopupView::GiftWaiting);
    EXPECT_EQ(popup.promptId(), world::PromptId::GiftWaiting);
}

}  // namespace
