#include <string>

#include "gtest/gtest.h"
#include "world/ConfigError.h"

namespace {


TEST(ConfigErrorTest, WhatIsThePathAsGiven) {
	const world::ConfigError error(world::ConfigErrorKind::InvalidValue,
				       "rooms.bedroom.elements[0].draw.mode");
	EXPECT_EQ(std::string(error.what()),
		  "rooms.bedroom.elements[0].draw.mode");
}

TEST(ConfigErrorTest, ExposesKind) {
	const world::ConfigError error(
		world::ConfigErrorKind::MissingStructure, "rooms.kitchen");
	EXPECT_EQ(error.kind(), world::ConfigErrorKind::MissingStructure);
	EXPECT_EQ(error.path(), "rooms.kitchen");
}

}
