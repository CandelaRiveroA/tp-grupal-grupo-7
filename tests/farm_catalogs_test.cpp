#include "gtest/gtest.h"

#include "world/CropConfigLoader.h"
#include "world/FarmCatalogs.h"
#include "world/InventoryConfigLoader.h"

#if defined(TALLER_INVENTORY_TOML) && defined(TALLER_CROPS_TOML)

namespace {

const world::CropCatalog loadCrops() {
	return world::CropConfigLoader().loadFromFile(TALLER_CROPS_TOML);
}

const world::InventoryLayout loadInventory() {
	return world::InventoryConfigLoader().loadFromFile(TALLER_INVENTORY_TOML);
}

TEST(FarmCatalogsTest, RegistersEverySackInTheInventoryContent) {
	const world::CropCatalog crops = loadCrops();
	const world::FarmCatalogs farm(crops, world::GroundCatalog(),
				       world::BuildPlaceCatalog());
	const world::InventoryLayout layout =
		farm.withCropContents(loadInventory());

	ASSERT_FALSE(crops.entries().empty());
	for (const auto& [cropId, definition] : crops.entries()) {
		EXPECT_NE(layout.content.find(definition.sackId), nullptr)
			<< definition.sackId;
		EXPECT_TRUE(layout.traits.isDroppable(definition.sackId))
			<< definition.sackId;
	}
}

TEST(FarmCatalogsTest, RegistersTheHarvestOfEveryCrop) {
	const world::CropCatalog crops = loadCrops();
	const world::FarmCatalogs farm(crops, world::GroundCatalog(),
				       world::BuildPlaceCatalog());
	const world::InventoryLayout layout =
		farm.withCropContents(loadInventory());

	for (const auto& [cropId, definition] : crops.entries()) {
		EXPECT_NE(layout.content.find(cropId), nullptr) << cropId;
	}
}

TEST(FarmCatalogsTest, LeavesTheBaseInventoryUntouched) {
	const world::CropCatalog crops = loadCrops();
	const world::FarmCatalogs farm(crops, world::GroundCatalog(),
				       world::BuildPlaceCatalog());
	const world::InventoryLayout base = loadInventory();
	const std::string sack = crops.entries().begin()->second.sackId;

	farm.withCropContents(base);

	EXPECT_EQ(base.content.find(sack), nullptr);
}

}

#endif
