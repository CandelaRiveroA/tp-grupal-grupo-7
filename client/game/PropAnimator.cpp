#include "game/PropAnimator.h"

#include <algorithm>

namespace game {

int PropAnimator::baseFrame(const State& state, const int iteration) const {
	const int last = state.frameCount - 1;
	return std::min((iteration - state.start) / state.frameTicks, last);
}

std::optional<PropMotion> PropAnimator::toggle(const std::string& id, const int frameCount,
			  const int frameTicks, const int iteration) {
	if (frameCount < 2 || frameTicks < 1) {
		return std::nullopt;
	}
	const auto found = states_.find(id);
	if (found != states_.end() &&
	    (iteration - found->second.start) / found->second.frameTicks <
		    found->second.frameCount - 1) {
		return std::nullopt;
	}
	State state;
	state.opening = found == states_.end() || !found->second.opening;
	state.start = iteration;
	state.frameCount = frameCount;
	state.frameTicks = frameTicks;
	states_[id] = state;
	return state.opening ? PropMotion::Opening : PropMotion::Closing;
}

void PropAnimator::reset(const std::string& id) {
	states_.erase(id);
}

bool PropAnimator::isOpen(const std::string& id, const int iteration) const {
	const auto found = states_.find(id);
	return found != states_.end() && found->second.opening &&
	       baseFrame(found->second, iteration) ==
		       found->second.frameCount - 1;
}

std::map<std::string, int> PropAnimator::frames(const int iteration) const {
	std::map<std::string, int> result;
	for (const auto& [id, state] : states_) {
		const int base = baseFrame(state, iteration);
		result[id] = state.opening ? base : state.frameCount - 1 - base;
	}
	return result;
}

}
