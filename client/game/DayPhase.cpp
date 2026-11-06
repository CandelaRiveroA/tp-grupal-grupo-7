#include "game/DayPhase.h"

#include <algorithm>
#include <iterator>

#include "game/GameConstants.h"

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
	  }
	, stageFromHour_{
		  {kClockStartHour, world::TimeOfDay::Day},
		  {18, world::TimeOfDay::Dusk},
		  {20, world::TimeOfDay::Evening},
		  {22, world::TimeOfDay::Night},
	  }
	, clockStage_(std::nullopt)
	, clockValues_{
		  {kClockWeekdayKey, 0},
		  {kClockDayKey, kFirstDayNumber},
		  {kClockHourKey, kClockStartHour},
		  {kClockMinuteKey, 0},
	  } {}

void DayPhase::setClock(const int dayNumber, const int minutesSinceStart) {
	const int hour = kClockStartHour + minutesSinceStart / kMinutesPerHour;
	clockValues_[kClockWeekdayKey] =
		(dayNumber - kFirstDayNumber) % kDaysPerWeek;
	clockValues_[kClockDayKey] = dayNumber;
	clockValues_[kClockHourKey] = hour;
	clockValues_[kClockMinuteKey] = minutesSinceStart % kMinutesPerHour;

	const world::TimeOfDay stage = stageForHour(hour);
	if (clockStage_ != stage) {
		clockStage_ = stage;
		setPhase(stage);
	}
}

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

const std::map<std::string, int>& DayPhase::clockValues() const {
	return clockValues_;
}

world::TimeOfDay DayPhase::stageForHour(const int hour) const {
	// La última etapa cuyo umbral ya pasó; antes del primero (no debería
	// pasar: el reloj arranca a las 6) rige el día.
	const auto after = stageFromHour_.upper_bound(hour);
	if (after == stageFromHour_.begin()) {
		return world::TimeOfDay::Day;
	}
	return std::prev(after)->second;
}

}  // namespace game
