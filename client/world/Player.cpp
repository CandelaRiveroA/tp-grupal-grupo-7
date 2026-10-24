#include "Player.h"

#include <algorithm>

namespace world {

Player::Player(int x, int y, int width, int height, int speed)
	: x_(x)
	, y_(y)
	, width_(width)
	, height_(height)
	, speed_(speed) {}

int Player::x() const { return x_; }

int Player::y() const { return y_; }

int Player::width() const { return width_; }

int Player::height() const { return height_; }

int Player::speed() const { return speed_; }

void Player::move(int dx, int dy, int worldWidth, int worldHeight) {

	x_ = std::clamp(x_ + dx, 0, std::max(0, worldWidth - width_));
	y_ = std::clamp(y_ + dy, 0, std::max(0, worldHeight - height_));
}

}
