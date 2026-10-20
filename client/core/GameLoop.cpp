#include "GameLoop.h"

namespace core {

GameLoop::GameLoop(std::function<void()> tick, int intervalMs)
	: tick_(std::move(tick)) {
	timer_.setInterval(intervalMs);

	QObject::connect(&timer_, &QTimer::timeout, [this]() { tick_(); });
}

void GameLoop::start() { timer_.start(); }

void GameLoop::stop() { timer_.stop(); }

}
