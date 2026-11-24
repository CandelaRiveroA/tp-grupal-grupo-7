#include "game/ChatCue.h"

namespace game {

bool ChatCue::update(const int incomingChatVersion) {
    const bool first = !seen_.has_value();
    const bool arrived = !first && incomingChatVersion != *seen_;
    seen_ = incomingChatVersion;
    return arrived;
}

}  // namespace game
