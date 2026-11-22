#include "game/MarketSetCycle.h"

#include <algorithm>

namespace game {

int MarketSetCycle::current() const {
	return current_;
}

void MarketSetCycle::next(int setCount) {
	current_ = (current_ + 1) % std::max(setCount, 1);
}

}
