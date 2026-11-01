#pragma once

#include <vector>

#include "world/SpriteVisual.h"
#include "world/ViewDirection.h"

namespace world {


enum class Layer {
    Body,
    Arms,
    Clothing,
    Accessory,
};

struct LayerFrame {
    Layer layer;
    SheetCell frame;
    int cols = 1;
    int rows = 1;
};

using Pose = std::vector<LayerFrame>;

enum class ActionKind {
    Idle,
    Walk,
    UseTool,
    Milk,
    Pickup,
};

struct ActionData {
    std::vector<Pose> frames;
    bool loops = false;
    int frameTicks = 0;
};

}  // namespace world
