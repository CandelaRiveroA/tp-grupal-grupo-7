#include "IndexCycle.h"

IndexCycle::IndexCycle(std::uint8_t count) : count_(count) {}

std::uint8_t IndexCycle::next(std::uint8_t current) const {
	if (count_ == 0) {
		return current;
	}
	return static_cast<std::uint8_t>((current + 1) % count_);
}

// Se suma count_ antes de restar para no operar en negativo: con uint8_t,
// current - 1 en el índice 0 daría 255.
std::uint8_t IndexCycle::previous(std::uint8_t current) const {
	if (count_ == 0) {
		return current;
	}
	return static_cast<std::uint8_t>((current + count_ - 1) % count_);
}
