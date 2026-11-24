#include "game/PopupState.h"

namespace game {

void PopupState::showQuestion() {
    giftSenderName_.clear();
    view_ = PopupView::Question;
    promptId_ = world::PromptId::SleepQuestion;
}

void PopupState::showWaiting(const int ready, const int total) {
    giftSenderName_.clear();
    // Solo, no hay a quién esperar: el server ya cierra el día y lo que
    // sigue es la pantalla negra con el OK. Mostrar "(1/1)" en el medio
    // sería un cartel sin sentido.
    if (total <= 1) {
        view_ = PopupView::Hidden;
        return;
    }
    waitingReady_ = ready;
    waitingTotal_ = total;
    view_ = PopupView::Waiting;
    promptId_ = world::PromptId::SleepWaiting;
}

void PopupState::showSleepState(const int sleeping, const int total,
                                const bool voted, const bool localDied) {
    if (sleeping == 0) {
        view_ = PopupView::Hidden;
        return;
    }
    if (voted || localDied) {
        showWaiting(sleeping, total);
        return;
    }
    // showQuestion fija también la COPIA de la pregunta: sin eso quedaba
    // la del popup anterior (p. ej. "¿Avanzamos al siguiente día?", que no
    // tiene opciones) y la pregunta salía sin Sí/No ni forma de contestar.
    showQuestion();
}

void PopupState::setWaitingCancelable(const bool cancelable) {
    waitingCancelable_ = cancelable;
}

bool PopupState::waitingCancelable() const { return waitingCancelable_; }

void PopupState::hide() { view_ = PopupView::Hidden; }

void PopupState::showNextDay() {
    giftSenderName_.clear();
    view_ = PopupView::NextDay;
    promptId_ = world::PromptId::NextDay;
}

void PopupState::showGiftOffer() {
    giftSenderName_.clear();
    view_ = PopupView::GiftOffer;
    promptId_ = world::PromptId::GiftOffer;
}

void PopupState::showGiftWaiting() {
    giftSenderName_.clear();
    view_ = PopupView::GiftWaiting;
    promptId_ = world::PromptId::GiftWaiting;
}

void PopupState::showGiftReceived(const std::string& senderName) {
    giftSenderName_ = senderName;
    view_ = PopupView::GiftReceived;
    promptId_ = world::PromptId::GiftReceived;
}

bool PopupState::modal() const { return view_ != PopupView::Hidden; }

PopupView PopupState::view() const { return view_; }

world::PromptId PopupState::promptId() const { return promptId_; }

const std::string& PopupState::giftSenderName() const {
    return giftSenderName_;
}

int PopupState::waitingReady() const { return waitingReady_; }

int PopupState::waitingTotal() const { return waitingTotal_; }

}  // namespace game
