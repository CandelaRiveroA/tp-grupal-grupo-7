#include <cassert>
#include <filesystem>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "world_fixtures.h"
#include "world_model/FormatError.h"
#include "world_model/Map.h"
#include "world_model/PrefabCatalog.h"
#include "world_model/PrefabCatalogLoader.h"
#include "world_model/WorldSerializer.h"

using worldmodel::FormatError;
using worldmodel::FormatErrorKind;
using worldmodel::Map;
using worldmodel::MapObject;
using worldmodel::ObjectRole;
using worldmodel::PrefabCatalog;
using worldmodel::PrefabCatalogLoader;
using worldmodel::PrefabDef;
using worldmodel::TileRef;
using worldmodel::World;
using worldmodel::WorldSerializer;

class PrefabCatalogLoaderTest : public ::testing::Test {
protected:
	static constexpr const char* kValidCatalogToml = R"(format_version = 1

[tilesets.prefab_arboles]
image = "World/prefabs/arboles.png"
cell = [16, 16]

[tilesets.prefab_caminos]
image = "World/prefabs/caminos.png"
cell = [16, 16]

[[prefabs]]
id = "roble"
name = "Roble"
category = "arboles"
category_label = "Arboles"
role = "cabin"
door = [1, 3]
tile = "prefab_arboles:0,0"
size = [2, 3]
collision_grid = [
	false, false,
	false, false,
	true, false,
]

[[prefabs]]
id = "camino_recto"
name = "Camino recto"
category = "caminos"
category_label = "Caminos"
tile = "prefab_caminos:1,2"
size = [2, 1]
collision_grid = [false, false]

[[prefabs]]
id = "pino"
name = "Pino"
category = "arboles"
category_label = "Arboles"
tile = "prefab_arboles:2,0"
size = [1, 2]
collision_grid = [false, true]
)";

	std::string replaced(std::string text, const std::string& from,
			     const std::string& to) const {
		const std::size_t pos = text.find(from);
		assert(pos != std::string::npos);
		text.replace(pos, from.size(), to);
		return text;
	}

	PrefabCatalog loadCatalogText(const std::string& toml) const {
		return PrefabCatalogLoader().loadFromString(toml);
	}

	void expectCatalogError(const std::string& toml, FormatErrorKind kind,
				const std::string& path) const {
		try {
			(void)loadCatalogText(toml);
			FAIL() << "expected FormatError at " << path;
		} catch (const FormatError& error) {
			EXPECT_EQ(error.kind(), kind) << error.what();
			EXPECT_EQ(error.path(), path);
		}
	}
};

TEST_F(PrefabCatalogLoaderTest, LoadsValidCatalogAndQueries) {
	const PrefabCatalog catalog = loadCatalogText(kValidCatalogToml);

	ASSERT_EQ(catalog.tilesets().size(), 2u);
	EXPECT_EQ(catalog.tilesets().at("prefab_arboles").image,
		  "World/prefabs/arboles.png");

	const auto categories = catalog.categories();
	ASSERT_EQ(categories.size(), 2u);
	EXPECT_EQ(categories[0],
		  (std::pair<std::string, std::string>{"arboles", "Arboles"}));
	EXPECT_EQ(categories[1],
		  (std::pair<std::string, std::string>{"caminos", "Caminos"}));

	const auto arboles = catalog.byCategory("arboles");
	ASSERT_EQ(arboles.size(), 2u);
	EXPECT_EQ(arboles[0]->id, "roble");
	EXPECT_EQ(arboles[1]->id, "pino");
	EXPECT_TRUE(catalog.byCategory("inexistente").empty());

	const PrefabDef* found = catalog.find("camino_recto");
	ASSERT_NE(found, nullptr);
	EXPECT_EQ(found->name, "Camino recto");
	EXPECT_EQ(found->tile, (TileRef{"prefab_caminos", 1, 2}));
	EXPECT_EQ(found->width, 2);
	EXPECT_EQ(found->height, 1);
	EXPECT_FALSE(found->role.has_value());
	EXPECT_FALSE(found->door.has_value());
	EXPECT_EQ(found->collisionGrid, (std::vector<bool>{false, false}));
	EXPECT_EQ(catalog.find("desconocido"), nullptr);

	const PrefabDef* cabin = catalog.findByRole(ObjectRole::Cabin);
	ASSERT_NE(cabin, nullptr);
	EXPECT_EQ(cabin->id, "roble");
	EXPECT_EQ(cabin->door, (worldmodel::TilePos{1, 3}));
	EXPECT_EQ(catalog.findByRole(ObjectRole::Carpentry), nullptr);
}

TEST_F(PrefabCatalogLoaderTest, RejectsInvalidVersion) {
	expectCatalogError(
		replaced(kValidCatalogToml, "format_version = 1",
			 "format_version = 2"),
		FormatErrorKind::InvalidValue, "format_version");
}

TEST_F(PrefabCatalogLoaderTest, RejectsDuplicateId) {
	expectCatalogError(
		replaced(kValidCatalogToml, "id = \"camino_recto\"",
			 "id = \"roble\""),
		FormatErrorKind::InvalidValue, "prefabs[1].id");
}

TEST_F(PrefabCatalogLoaderTest, RejectsUnknownTileset) {
	expectCatalogError(
		replaced(kValidCatalogToml, "tile = \"prefab_arboles:0,0\"",
			 "tile = \"no_existe:0,0\""),
		FormatErrorKind::InvalidValue, "prefabs[0].tile");
}

TEST_F(PrefabCatalogLoaderTest, RejectsNonPositiveSize) {
	expectCatalogError(
		replaced(kValidCatalogToml, "size = [2, 3]", "size = [0, 3]"),
		FormatErrorKind::InvalidValue, "prefabs[0].size");
}

TEST_F(PrefabCatalogLoaderTest, RejectsMismatchedCollisionGridSize) {
	expectCatalogError(
		replaced(kValidCatalogToml,
			 "collision_grid = [\n\tfalse, false,\n\tfalse, "
			 "false,\n\ttrue, false,\n]",
			 "collision_grid = [true, false]"),
		FormatErrorKind::InvalidValue, "prefabs[0].collision_grid");
}

TEST_F(PrefabCatalogLoaderTest, RejectsInvalidRoleOrDoor) {
	expectCatalogError(
		replaced(kValidCatalogToml, "role = \"cabin\"",
			 "role = \"decoration\""),
		FormatErrorKind::InvalidValue, "prefabs[0].role");
	expectCatalogError(
		replaced(kValidCatalogToml, "role = \"cabin\"",
			 "role = \"invalido\""),
		FormatErrorKind::InvalidValue, "prefabs[0].role");
	expectCatalogError(
		replaced(kValidCatalogToml, "door = [1, 3]", "door = [2, 3]"),
		FormatErrorKind::InvalidValue, "prefabs[0].door");
	expectCatalogError(
		replaced(kValidCatalogToml, "door = [1, 3]", "door = [1, 4]"),
		FormatErrorKind::InvalidValue, "prefabs[0].door");
}

TEST_F(PrefabCatalogLoaderTest, LoadsRealPrefabsTomlIfPresent) {
	if (!std::filesystem::exists(TALLER_PREFABS_TOML)) {
		return;
	}
	const PrefabCatalog catalog =
		PrefabCatalogLoader().loadFromFile(TALLER_PREFABS_TOML);
	EXPECT_FALSE(catalog.categories().empty());
	EXPECT_FALSE(catalog.byCategory(catalog.categories().front().first)
			     .empty());

	const PrefabDef* cabin = catalog.findByRole(ObjectRole::Cabin);
	ASSERT_NE(cabin, nullptr);
	EXPECT_EQ(cabin->id, "casa_granja_1");
	EXPECT_EQ(cabin->door, (worldmodel::TilePos{5, 7}));

	const PrefabDef* center =
		catalog.findByRole(ObjectRole::CommunityCenter);
	ASSERT_NE(center, nullptr);
	EXPECT_EQ(center->id, "centro_comunitario");
}

TEST(MapCollisionGridTest, ComposeCollisionUsesPartialCollisionGrid) {
	Map map;
	map.id = "farm";
	map.width = 4;
	map.height = 3;
	map.baseCollision.assign(12, false);
	map.baseCollision[0] = true;

	MapObject tree;
	tree.role = ObjectRole::Decoration;
	tree.tile = TileRef{"outdoors", 0, 0};
	tree.cell = {1, 0};
	tree.width = 2;
	tree.height = 3;
	tree.blocked = false;
	tree.collisionGrid = {
		false, false,
		false, false,
		true,  false,
	};
	map.objects.push_back(tree);

	const std::vector<bool> composed = map.composeCollision();
	ASSERT_EQ(composed.size(), 12u);
	EXPECT_TRUE(composed[0]);
	EXPECT_FALSE(composed[0 * 4 + 1]);
	EXPECT_FALSE(composed[0 * 4 + 2]);
	EXPECT_FALSE(composed[1 * 4 + 1]);
	EXPECT_FALSE(composed[1 * 4 + 2]);
	EXPECT_TRUE(composed[2 * 4 + 1]);
	EXPECT_FALSE(composed[2 * 4 + 2]);
}

TEST(MapCollisionGridTest, ComposeCollisionKeepsBlockedFallbackWhenEmpty) {
	Map map;
	map.id = "farm";
	map.width = 4;
	map.height = 2;
	map.baseCollision.assign(8, false);

	MapObject solid;
	solid.role = ObjectRole::Decoration;
	solid.tile = TileRef{"outdoors", 0, 0};
	solid.cell = {0, 0};
	solid.width = 2;
	solid.height = 1;
	solid.blocked = true;
	map.objects.push_back(solid);

	MapObject passable;
	passable.role = ObjectRole::Decoration;
	passable.tile = TileRef{"outdoors", 0, 0};
	passable.cell = {2, 0};
	passable.width = 2;
	passable.height = 1;
	passable.blocked = false;
	map.objects.push_back(passable);

	const std::vector<bool> composed = map.composeCollision();
	EXPECT_TRUE(composed[0]);
	EXPECT_TRUE(composed[1]);
	EXPECT_FALSE(composed[2]);
	EXPECT_FALSE(composed[3]);
}

TEST(WorldSerializerCollisionGridTest, RoundTripPreservesObjectsWithAndWithoutGrid) {
	World world = worldfixtures::loadMinimalWorld();
	Map& town = world.maps.at("town");

	MapObject withGrid;
	withGrid.role = ObjectRole::Decoration;
	withGrid.tile = TileRef{"outdoors", 1, 1};
	withGrid.cell = {0, 0};
	withGrid.width = 2;
	withGrid.height = 2;
	withGrid.blocked = true;
	withGrid.collisionGrid = {false, false, true, false};
	town.objects.push_back(withGrid);

	MapObject withoutGrid;
	withoutGrid.role = ObjectRole::Decoration;
	withoutGrid.tile = TileRef{"outdoors", 2, 1};
	withoutGrid.cell = {2, 0};
	withoutGrid.width = 1;
	withoutGrid.height = 1;
	withoutGrid.blocked = false;
	town.objects.push_back(withoutGrid);

	town.rebuildCollision();

	const WorldSerializer serializer;
	const std::string saved = serializer.saveToString(world);
	const World reloaded = serializer.loadFromString(saved);

	const Map& reloadedTown = reloaded.maps.at("town");
	ASSERT_EQ(reloadedTown.objects.size(), 2u);
	EXPECT_EQ(reloadedTown.objects[0].collisionGrid,
		  (std::vector<bool>{false, false, true, false}));
	EXPECT_TRUE(reloadedTown.objects[1].collisionGrid.empty());
	EXPECT_EQ(reloadedTown.collision, town.composeCollision());
}

TEST(WorldSerializerCollisionGridTest, RejectsObjectCollisionGridSizeMismatch) {
	const std::string badWorld = R"(format_version = 1

[world]
name = "X"
max_players = 1

[tilesets.outdoors]
image = "a.png"
cell = [16, 16]

[maps.farm]
kind = "farm"
width = 2
height = 2
tile_size = 16
base_collision = [false, false, false, false]
collision = [false, false, false, false]
ground = ["outdoors:0,0", "outdoors:0,0", "outdoors:0,0", "outdoors:0,0"]

[[maps.farm.objects]]
role = "decoration"
tile = "outdoors:0,0"
cell = [0, 0]
size = [2, 1]
collision_grid = [true, false, true]
)";
	try {
		(void)WorldSerializer().loadFromString(badWorld);
		FAIL() << "expected FormatError";
	} catch (const FormatError& error) {
		EXPECT_EQ(error.kind(), FormatErrorKind::InvalidValue);
		EXPECT_EQ(error.path(), "maps.farm.objects[0].collision_grid");
	}
}
