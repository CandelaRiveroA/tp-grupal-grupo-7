#include "world/DigitCatalog.h"

#include <algorithm>

namespace world {

const SheetCell& DigitCatalog::cellFor(int digit) const {
	const int clamped = std::clamp(digit, 0, 9);
	return digits[static_cast<std::size_t>(clamped)];
}

std::vector<int> digitsOf(int n) {
	if (n <= 0) {
		return {0};
	}
	std::vector<int> reversed;
	while (n > 0) {
		reversed.push_back(n % 10);
		n /= 10;
	}
	std::reverse(reversed.begin(), reversed.end());
	return reversed;
}

}  // namespace world
