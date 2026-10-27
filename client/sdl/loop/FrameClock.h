#pragma once

#include <chrono>

namespace sdl {

class FrameClock {
public:
	using Clock = std::chrono::steady_clock;
	using TimePoint = Clock::time_point;
	using Duration = Clock::duration;

	explicit FrameClock(int fps);

	explicit FrameClock(Duration rate);

	void restart(TimePoint now);

	int iteration() const;

	Duration advance(TimePoint now);

private:
	Duration rate_;
	TimePoint t1_;
	int iteration_;
};

}
