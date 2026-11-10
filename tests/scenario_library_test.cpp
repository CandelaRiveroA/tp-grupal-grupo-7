#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "world_model/FormatError.h"
#include "world_model/ScenarioLibrary.h"

namespace {

using worldmodel::FormatError;
using worldmodel::FormatErrorKind;
using worldmodel::MapKind;
using worldmodel::Scenario;
using worldmodel::ScenarioLibrary;

Scenario makeScenario(const std::string& id, MapKind kind) {
	Scenario scenario;
	scenario.id = id;
	scenario.kind = kind;
	return scenario;
}

TEST(ScenarioLibraryTest, AddFindAndAll) {
	ScenarioLibrary library;
	library.add(makeScenario("farm_a", MapKind::Farm));
	library.add(makeScenario("farm_b", MapKind::Farm));
	library.add(makeScenario("town_a", MapKind::Town));

	ASSERT_EQ(library.all().size(), 3u);
	ASSERT_NE(library.find("farm_b"), nullptr);
	EXPECT_EQ(library.find("farm_b")->id, "farm_b");
	EXPECT_EQ(library.find("nope"), nullptr);
}

TEST(ScenarioLibraryTest, RejectsDuplicateId) {
	ScenarioLibrary library;
	library.add(makeScenario("farm_a", MapKind::Farm));
	try {
		library.add(makeScenario("farm_a", MapKind::Town));
		FAIL() << "expected FormatError";
	} catch (const FormatError& error) {
		EXPECT_EQ(error.kind(), FormatErrorKind::InvalidValue);
		EXPECT_EQ(error.path(), "scenario");
	}
}

TEST(ScenarioLibraryTest, RejectsEmptyId) {
	ScenarioLibrary library;
	try {
		library.add(makeScenario("", MapKind::Farm));
		FAIL() << "expected FormatError";
	} catch (const FormatError& error) {
		EXPECT_EQ(error.kind(), FormatErrorKind::InvalidValue);
		EXPECT_EQ(error.path(), "scenario");
	}
}

TEST(ScenarioLibraryTest, FiltersByKind) {
	ScenarioLibrary library;
	library.add(makeScenario("farm_a", MapKind::Farm));
	library.add(makeScenario("farm_b", MapKind::Farm));
	library.add(makeScenario("town_a", MapKind::Town));

	const std::vector<const Scenario*> farms =
		library.byKind(MapKind::Farm);
	ASSERT_EQ(farms.size(), 2u);
	EXPECT_EQ(farms[0]->id, "farm_a");
	EXPECT_EQ(farms[1]->id, "farm_b");
	EXPECT_EQ(library.byKind(MapKind::Town).size(), 1u);
	EXPECT_TRUE(library.byKind(MapKind::Dungeon).empty());
}

}
