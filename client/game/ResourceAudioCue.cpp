#include "game/ResourceAudioCue.h"

namespace game {

std::vector<ResourceAudioEvent> ResourceAudioCue::update(
        const std::vector<ViewResourceHit>& hits) const {
    std::vector<ResourceAudioEvent> events;
    events.reserve(hits.size());
    for (const ViewResourceHit& hit: hits) {
        events.push_back(hit.destroyed ? ResourceAudioEvent::Broke : ResourceAudioEvent::Hit);
    }
    return events;
}

}  // namespace game
