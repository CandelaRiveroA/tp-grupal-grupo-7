#pragma once

#include <cstdint>


class IndexCycle {
public:
	explicit IndexCycle(std::uint8_t count);

	std::uint8_t next(std::uint8_t current) const;
	std::uint8_t previous(std::uint8_t current) const;

private:
	std::uint8_t count_;
};
