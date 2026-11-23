#include "game/PopupState.h"

namespace game {

void PopupState::showQuestion() {
    giftSenderName_.clear();
    view_ = PopupView::Question;
    promptId_ = world::PromptId::SleepQuestion;
}

void PopupState::showWaiting(const int ready, const int total) {
    giftSenderName_.clear();
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
