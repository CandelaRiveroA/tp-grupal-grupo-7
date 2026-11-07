#include "game/PopupState.h"

namespace game {

void PopupState::showQuestion() { view_ = PopupView::Question; }

void PopupState::showWaiting(const int ready, const int total) {
    waitingReady_ = ready;
    waitingTotal_ = total;
    view_ = PopupView::Waiting;
}

void PopupState::hide() { view_ = PopupView::Hidden; }

void PopupState::showNextDay() { view_ = PopupView::NextDay; }

bool PopupState::modal() const { return view_ != PopupView::Hidden; }

PopupView PopupState::view() const { return view_; }

int PopupState::waitingReady() const { return waitingReady_; }

int PopupState::waitingTotal() const { return waitingTotal_; }

}  // namespace game
