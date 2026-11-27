#include "game/NoticeQueue.h"

namespace game {

NoticeQueue::NoticeQueue(int durationTicks): durationTicks_(durationTicks > 0 ? durationTicks : 1) {}

void NoticeQueue::show(const std::string& id) { active_ = ActiveNotice{id, durationTicks_}; }

void NoticeQueue::observe(const std::vector<ViewNotice>& notices) {
    for (const ViewNotice& notice: notices) {
        show(notice.id);
    }
}

void NoticeQueue::tick() {
    if (!active_.has_value()) {
        return;
    }
    if (--active_->remainingTicks <= 0) {
        active_.reset();
    }
}

const std::optional<ActiveNotice>& NoticeQueue::active() const { return active_; }

}  // namespace game
