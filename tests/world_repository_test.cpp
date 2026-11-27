#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "world_fixtures.h"
#include "world_model/FormatError.h"
#include "world_model/ValidationIssue.h"
#include "world_model/WorldRepository.h"

namespace {

using worldfixtures::defaultWorld;
using worldmodel::FormatError;
using worldmodel::FormatErrorKind;
using worldmodel::ValidationIssue;
using worldmodel::World;
using worldmodel::WorldName;
using worldmodel::WorldRepository;

std::filesystem::path baseDir() {
	return std::filesystem::temp_directory_path() /
	       "taller_world_repository_test";
}

std::filesystem::path worldsDir() {
	return baseDir() / "mundos";
}

class WorldRepositoryTest : public ::testing::Test {
protected:
	void SetUp() override {
		std::filesystem::remove_all(baseDir());
	}
	void TearDown() override {
		std::filesystem::remove_all(baseDir());
	}
};

TEST_F(WorldRepositoryTest, ListWorldsWithoutDirectoryIsEmpty) {
	WorldRepository repository(worldsDir().string());
	EXPECT_TRUE(repository.listWorlds().empty());
}

TEST_F(WorldRepositoryTest, SaveIfValidWritesCreatesDirectoryAndListed) {
	const WorldRepository repository(worldsDir().string());
	const World world = defaultWorld();

	EXPECT_TRUE(repository.saveIfValid(world).empty());
	EXPECT_TRUE(std::filesystem::is_regular_file(repository.filePathFor(
		"Primavera")));
	const std::filesystem::path farmMinimap(
		repository.minimapPathFor("Primavera", "farm"));
	EXPECT_EQ(farmMinimap,
		  worldsDir() / "Primavera_minimaps" / "farm.png");
	std::filesystem::create_directories(farmMinimap.parent_path());
	std::ofstream(farmMinimap).close();

	const std::vector<WorldName> worlds = repository.listWorlds();
	ASSERT_EQ(worlds.size(), 1u);
	EXPECT_EQ(worlds[0], "Primavera");

	EXPECT_EQ(repository.load("Primavera").name, "Primavera");
}

TEST_F(WorldRepositoryTest, SaveIfValidRejectsInvalidWorldWithoutWriting) {
	const WorldRepository repository(worldsDir().string());
	World world = defaultWorld();
	world.maps.at("farm").objects.clear();

	const std::vector<ValidationIssue> issues =
		repository.saveIfValid(world);
	EXPECT_FALSE(issues.empty());
	EXPECT_FALSE(std::filesystem::exists(repository.filePathFor(
		"Primavera")));
	EXPECT_TRUE(repository.listWorlds().empty());

	try {
		(void)repository.load("no-such-world");
		FAIL() << "expected FormatError";
	} catch (const FormatError& error) {
		EXPECT_EQ(error.kind(), FormatErrorKind::UnreadableInput);
	}
}

TEST_F(WorldRepositoryTest, LoadRejectsInvalidName) {
	const WorldRepository repository(worldsDir().string());
	try {
		(void)repository.load("no/such");
		FAIL() << "expected FormatError";
	} catch (const FormatError& error) {
		EXPECT_EQ(error.kind(), FormatErrorKind::InvalidValue);
		EXPECT_EQ(error.path(), "name");
	}
}

TEST_F(WorldRepositoryTest, ListWorldsReportsCorruptFiles) {
	const WorldRepository repository(worldsDir().string());
	EXPECT_TRUE(repository.saveIfValid(defaultWorld()).empty());
	const std::filesystem::path broken =
		worldsDir() / "broken.toml";
	std::ofstream stream(broken);
	stream << "esto no es un toml de mundo";
	stream.close();

	const std::vector<WorldName> worlds = repository.listWorlds();
	ASSERT_EQ(worlds.size(), 2u);
	EXPECT_EQ(worlds[0], "Primavera");
	EXPECT_EQ(worlds[1], "broken");
}

TEST_F(WorldRepositoryTest, PrepareMinimapsDirPrunesStaleMapsAndKeepsValid) {
	const WorldRepository repository(worldsDir().string());
	const World world = defaultWorld();
	const std::filesystem::path farmPng(
		repository.minimapPathFor(world.name, "farm"));
	const std::filesystem::path stalePng(
		repository.minimapPathFor(world.name, "extra_borrado"));
	std::filesystem::create_directories(farmPng.parent_path());
	std::ofstream(farmPng).close();
	std::ofstream(stalePng).close();

	EXPECT_TRUE(repository.prepareMinimapsDir(world));
	EXPECT_TRUE(std::filesystem::is_regular_file(farmPng));
	EXPECT_FALSE(std::filesystem::exists(stalePng));
}

}  // namespace
