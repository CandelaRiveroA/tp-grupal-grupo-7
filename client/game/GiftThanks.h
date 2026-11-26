#pragma once

#include <cstdint>
#include <map>
#include <vector>

#include "game/LocalWorldView.h"

namespace game {

// Duración visual del agradecimiento, por ID estable y para cualquier jugador.
class GiftThanks {
private:
    std::map<int, int> remaining_;

public:
    void update(const std::vector<uint8_t>& acceptedPlayers, int durationTicks);
    void apply(LocalWorldView& view) const;
};

}  // namespace game
