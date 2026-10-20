#pragma once

namespace core {
class WindowConfig;
}

namespace world {
class Player;
}

namespace world {

struct Color {
	int r;
	int g;
	int b;
	int a;
};

class Scenario {
public:
	Scenario(const core::WindowConfig& config, const Player& player);

	int width() const;
	int height() const;
	const Color& background() const;
	const Player& player() const;

private:
	int width_;
	int height_;
	Color background_;
	const Player* player_;
};

}
