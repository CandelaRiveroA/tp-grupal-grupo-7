#include "game/SceneSnapshot.h"

namespace game {

const LocalWorldView& SceneSnapshot::observe(const LocalWorldView& incoming,
                                            const std::string& sceneId) {
    if (incoming.mapId.empty() || incoming.mapId == sceneId) {
        visible_ = incoming;
    }
    return visible_;
}

}  // namespace game
