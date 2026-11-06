#include "game/DayPhase.h"

#include <algorithm>

namespace game {

DayPhase::DayPhase(const int transitionTicks)
	: phase_(world::TimeOfDay::Day)
	, level_(0.0)
	, step_(1.0 / std::max(1, transitionTicks))
	, nightFactors_{
		  {world::TimeOfDay::Day, 0.0},
		  {world::TimeOfDay::Dusk, 0.35},
		  {world::TimeOfDay::Evening, 0.7},
		  {world::TimeOfDay::Night, 1.0},
	  }
	, lookPhases_{
		  {world::TimeOfDay::Day, world::TimeOfDay::Day},
		  {world::TimeOfDay::Dusk, world::TimeOfDay::Day},
		  {world::TimeOfDay::Evening, world::TimeOfDay::Night},
		  {world::TimeOfDay::Night, world::TimeOfDay::Night},
	  } {}

void DayPhase::setPhase(const world::TimeOfDay phase) {
	phase_ = phase;
}

void DayPhase::advance() {
	const double gap = nightFactors_.at(phase_) - level_;
	level_ += std::clamp(gap, -step_, step_);
}

world::TimeOfDay DayPhase::phase() const {
	return lookPhases_.at(phase_);
}

double DayPhase::nightFactor() const {
	return level_;
}

}  // namespace game
