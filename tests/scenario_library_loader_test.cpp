#include <string>

#include "gtest/gtest.h"
#include "world_model/ScenarioLibrary.h"
#include "world_model/ScenarioLibraryLoader.h"

namespace {

using worldmodel::MapKind;
using worldmodel::ScenarioLibrary;
using worldmodel::ScenarioLibraryLoader;

TEST(ScenarioLibraryLoaderTest, LoadsTomlFilesInOrder) {
	const ScenarioLibrary library =
		ScenarioLibraryLoader().loadFromDirectory(
			TALLER_SCENARIO_FIXTURES_DIR);

	const auto& all = library.all();
	ASSERT_EQ(all.size(), 2u);
	EXPECT_EQ(all[0].id, "farm_a");
	EXPECT_EQ(all[0].kind, MapKind::Farm);
	EXPECT_EQ(all[1].id, "town_a");
	EXPECT_EQ(all[1].kind, MapKind::Town);

	ASSERT_NE(library.find("farm_a"), nullptr);
	EXPECT_EQ(library.find("farm_a")->width, 2);
}

}
