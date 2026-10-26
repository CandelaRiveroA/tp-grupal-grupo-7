#include "client/qt/util/CharacterLayers.h"

#include <gtest/gtest.h>

TEST(CharacterLayersTest, ValueDevuelveElCampoDeCadaCapa) {
	CharacterAppearance appearance;
	appearance.skin = 3;
	appearance.eyes = 2;
	appearance.hair = 5;
	appearance.hairColor = 4;
	appearance.shirt = 1;
	appearance.pants = 0;
	appearance.accessory = 2;

	const CharacterLayers layers;
	EXPECT_EQ(std::uint8_t{3},
		  layers.value(appearance, CharacterLayer::Skin));
	EXPECT_EQ(std::uint8_t{2},
		  layers.value(appearance, CharacterLayer::Eyes));
	EXPECT_EQ(std::uint8_t{5},
		  layers.value(appearance, CharacterLayer::Hair));
	EXPECT_EQ(std::uint8_t{4},
		  layers.value(appearance, CharacterLayer::HairColor));
	EXPECT_EQ(std::uint8_t{1},
		  layers.value(appearance, CharacterLayer::Shirt));
	EXPECT_EQ(std::uint8_t{0},
		  layers.value(appearance, CharacterLayer::Pants));
	EXPECT_EQ(std::uint8_t{2},
		  layers.value(appearance, CharacterLayer::Accessory));
}

TEST(CharacterLayersTest, SetEscribeSoloLaCapaIndicada) {
	CharacterAppearance appearance;
	const CharacterLayers layers;

	layers.set(appearance, CharacterLayer::Accessory, 3);

	EXPECT_EQ(std::uint8_t{3},
		  layers.value(appearance, CharacterLayer::Accessory));
	EXPECT_EQ(std::uint8_t{0},
		  layers.value(appearance, CharacterLayer::Skin));
	EXPECT_EQ(std::uint8_t{0},
		  layers.value(appearance, CharacterLayer::Eyes));
	EXPECT_EQ(std::uint8_t{0},
		  layers.value(appearance, CharacterLayer::Hair));
	EXPECT_EQ(std::uint8_t{0},
		  layers.value(appearance, CharacterLayer::HairColor));
	EXPECT_EQ(std::uint8_t{0},
		  layers.value(appearance, CharacterLayer::Shirt));
	EXPECT_EQ(std::uint8_t{0},
		  layers.value(appearance, CharacterLayer::Pants));
}

TEST(CharacterLayersTest, ValueYSetHacenRoundTripEnTodasLasCapas) {
	CharacterAppearance appearance;
	const CharacterLayers layers;

	for (int layer = 0; layer < static_cast<int>(CharacterLayer::Count);
	     ++layer) {
		const auto current = static_cast<CharacterLayer>(layer);
		const auto index = static_cast<std::uint8_t>(layer + 1);

		layers.set(appearance, current, index);
		EXPECT_EQ(index, layers.value(appearance, current));
	}
}
