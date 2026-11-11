#include "game/FractionStat.h"

#include <algorithm>

namespace game {

void FractionStat::spend(const double cost) {
	fraction_ = std::clamp(fraction_ - std::max(0.0, cost), 0.0, 1.0);
}

void FractionStat::reset() { fraction_ = 1.0; }

double FractionStat::fraction() const { return fraction_; }

}
