#pragma once

#include <string>

#include "game/LocalWorldView.h"

namespace game {

// Conserva los datos de la escena visible hasta cambiar el fondo en negro.
class SceneSnapshot {
public:
    const LocalWorldView& observe(const LocalWorldView& incoming, const std::string& sceneId);

private:
    LocalWorldView visible_;
};

}  // namespace game
