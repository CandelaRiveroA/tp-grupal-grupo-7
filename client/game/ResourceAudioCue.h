#pragma once

#include <vector>

#include "game/LocalWorldView.h"

namespace game {

enum class ResourceAudioEvent {
    Hit,
    Broke,
};

// Traduce impactos confirmados a audio; no infiere golpes del estado visual.
class ResourceAudioCue {
public:
    std::vector<ResourceAudioEvent> update(const std::vector<ViewResourceHit>& hits) const;
};

}  // namespace game
