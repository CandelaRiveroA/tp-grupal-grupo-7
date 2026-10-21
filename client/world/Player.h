#pragma once

namespace world {

class Player {
public:
	Player(int x, int y, int width, int height, int speed);

	int x() const;
	int y() const;
	int width() const;
	int height() const;
	int speed() const;

	void move(int dx, int dy, int worldWidth, int worldHeight);

private:
	int x_;
	int y_;
	int width_;
	int height_;
	int speed_;
};

}
