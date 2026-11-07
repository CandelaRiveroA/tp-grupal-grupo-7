#include "common_frame_clock.h"

#include <chrono>

namespace sdl {

FrameClock::FrameClock(int fps)
	: FrameClock(std::chrono::duration_cast<Duration>(
			  std::chrono::seconds(1)) /
		      fps) {}

FrameClock::FrameClock(Duration rate)
	: rate_(rate)
	, t1_()
	, iteration_(0) {}

void FrameClock::restart(TimePoint now) {
	t1_ = now;
	iteration_ = 0;
}

int FrameClock::iteration() const { return iteration_; }

FrameClock::Duration FrameClock::advance(TimePoint now) {
	const Duration elapsed = now - t1_;
	const Duration rest = rate_ - elapsed;

	if (rest >= Duration::zero()) {
		t1_ += rate_;
		iteration_ += 1;
		return rest;
	}

	const Duration behind = -rest;
	const Duration lost = behind - behind % rate_;
	t1_ += lost;
	iteration_ += static_cast<int>(lost / rate_);

	t1_ += rate_;
	iteration_ += 1;

	return Duration::zero();
}

}
