#include "AsyncStateView.h"

#include <algorithm>

AsyncStateView::AsyncStateView() = default;

void AsyncStateView::bind(ScreenState state, QWidget* widget) {
	bindings_[widget].push_back(state);
}

void AsyncStateView::setState(ScreenState state) const {
	for (const auto& binding : bindings_) {
		const auto& states = binding.second;
		const bool shows =
			std::find(states.begin(), states.end(), state) !=
			states.end();
		binding.first->setVisible(shows);
	}
}
