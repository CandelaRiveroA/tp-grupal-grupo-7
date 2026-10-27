#include "game/ObtainedBannerQueue.h"

#include <algorithm>

namespace game {

ObtainedBannerQueue::ObtainedBannerQueue(int durationTicks, int maxVisible):
        durationTicks_(durationTicks > 0 ? durationTicks : 1),
        maxVisible_(maxVisible > 0 ? maxVisible : 1) {}

void ObtainedBannerQueue::push(world::ResourceKind kind) {
    if (static_cast<int>(banners_.size()) >= maxVisible_) {
        return;  // pila llena: se descarta el evento nuevo
    }
    const int stackIndex = static_cast<int>(banners_.size());
    banners_.push_back(ActiveBanner{kind, durationTicks_, stackIndex});
}

void ObtainedBannerQueue::observe(const std::vector<ViewResourceObtained>& obtained) {
    for (const ViewResourceObtained& event: obtained) {
        push(event.kind);
    }
}

void ObtainedBannerQueue::tick() {
    for (ActiveBanner& banner: banners_) {
        if (banner.remainingTicks > 0) {
            --banner.remainingTicks;
        }
    }
    banners_.erase(std::remove_if(banners_.begin(), banners_.end(),
                                  [](const ActiveBanner& b) { return b.remainingTicks <= 0; }),
                   banners_.end());
    reindex();
}

void ObtainedBannerQueue::reindex() {
    for (std::size_t i = 0; i < banners_.size(); ++i) {
        banners_[i].stackIndex = static_cast<int>(i);
    }
}

const std::vector<ActiveBanner>& ObtainedBannerQueue::active() const { return banners_; }

}  // namespace game
