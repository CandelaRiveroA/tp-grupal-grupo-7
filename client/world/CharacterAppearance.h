#pragma once

#include <vector>

#include "common/common_game_direction.h"

namespace world {


struct FrameIndex {
	int col;
	int row;
};

enum class Layer {
	Body,
	Arms,
	Clothing,
	Accessory,
};

using Direction = protocol::Direction;

struct LayerFrame {
	Layer layer;
	FrameIndex frame;
	int cols = 1;
	int rows = 1;
};

using Pose = std::vector<LayerFrame>;

enum class ActionKind {
	Idle,
	Walk,
	UseTool,
};

struct ActionData {
	std::vector<Pose> frames;
	bool loops = false;
	int frameTicks = 0;
};

}
