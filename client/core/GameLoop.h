#pragma once

#include <QTimer>

#include <functional>

namespace core {

class GameLoop {
public:
	explicit GameLoop(std::function<void()> tick, int intervalMs = 16);

	void start();
	void stop();

private:
	QTimer timer_;
	std::function<void()> tick_;
};

}
