#include "game/PopupState.h"

namespace game {

void PopupState::showQuestion() { view_ = PopupView::Question; }

void PopupState::showWaiting(const int ready, const int total) {
    waitingReady_ = ready;
    waitingTotal_ = total;
    view_ = PopupView::Waiting;
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
    view_ = PopupView::Question;
}

void PopupState::setWaitingCancelable(const bool cancelable) {
    waitingCancelable_ = cancelable;
}

bool PopupState::waitingCancelable() const { return waitingCancelable_; }

void PopupState::hide() { view_ = PopupView::Hidden; }

void PopupState::showNextDay() { view_ = PopupView::NextDay; }

bool PopupState::modal() const { return view_ != PopupView::Hidden; }

PopupView PopupState::view() const { return view_; }

int PopupState::waitingReady() const { return waitingReady_; }

int PopupState::waitingTotal() const { return waitingTotal_; }

}  // namespace game
