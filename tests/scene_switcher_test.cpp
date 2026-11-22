#include <set>
#include <string>
#include <utility>
#include <vector>

#include "gtest/gtest.h"
#include "world/ConfigError.h"
#include "world/SceneSource.h"
#include "world/SceneSwitcher.h"

namespace {

// Fuente falsa: conoce un conjunto de ids y marca cada escena con su
// origen en `cellPixelSize` para distinguir quién la armó.
class FakeSource : public world::SceneSource {
public:
	FakeSource(std::set<std::string> ids, int marker)
		: ids_(std::move(ids)), marker_(marker) {}

	std::vector<std::string> ids() const override {
		return std::vector<std::string>(ids_.begin(), ids_.end());
	}

	bool has(const std::string& id) const override {
		return ids_.count(id) == 1;
	}

	world::SceneDescription load(const std::string&) const override {
		world::SceneDescription scene;
		scene.cellPixelSize = marker_;
		return scene;
	}

private:
	std::set<std::string> ids_;
	int marker_;
};

world::SceneDescription initialScene() {
	world::SceneDescription scene;
	scene.cellPixelSize = 1;
	return scene;
}

TEST(SceneSwitcherTest, StartsWithTheInitialScene) {
	const FakeSource source({"bedroom"}, 10);
	const world::SceneSwitcher switcher({&source}, "bedroom",
					    initialScene());
	EXPECT_EQ(switcher.id(), "bedroom");
	EXPECT_EQ(switcher.scene().cellPixelSize, 1);
}

TEST(SceneSwitcherTest, SwitchLoadsTheSceneFromTheSourceThatKnowsIt) {
	const FakeSource source({"bedroom", "market"}, 10);
	world::SceneSwitcher switcher({&source}, "bedroom", initialScene());
	switcher.switchTo("market");
	EXPECT_EQ(switcher.id(), "market");
	EXPECT_EQ(switcher.scene().cellPixelSize, 10);
}

TEST(SceneSwitcherTest, FirstSourceThatHasTheIdWins) {
	const FakeSource editor({"market"}, 20);
	const FakeSource rooms({"market", "bedroom"}, 10);
	world::SceneSwitcher switcher({&editor, &rooms}, "bedroom",
				      initialScene());
	switcher.switchTo("market");
	EXPECT_EQ(switcher.scene().cellPixelSize, 20);
	switcher.switchTo("bedroom");
	EXPECT_EQ(switcher.scene().cellPixelSize, 10);
}

TEST(SceneSwitcherTest, UnknownIdThrowsAndKeepsTheCurrentScene) {
	const FakeSource source({"bedroom"}, 10);
	world::SceneSwitcher switcher({&source}, "bedroom", initialScene());
	try {
		switcher.switchTo("kitchen");
		FAIL() << "se esperaba ConfigError";
	} catch (const world::ConfigError& error) {
		EXPECT_EQ(error.kind(),
			  world::ConfigErrorKind::MissingStructure);
	}
	EXPECT_EQ(switcher.id(), "bedroom");
	EXPECT_EQ(switcher.scene().cellPixelSize, 1);
}

TEST(SceneSwitcherTest, NextIdWalksAllSourcesWithoutRepeatingAndWraps) {
	const FakeSource editor({"farm", "market"}, 20);
	const FakeSource rooms({"bedroom", "market"}, 10);
	world::SceneSwitcher switcher({&editor, &rooms}, "bedroom",
				      initialScene());
	// Orden: ids del editor (farm, market) y luego los de salas que
	// no estaban (bedroom).
	EXPECT_EQ(switcher.nextId(), "farm");
	switcher.switchTo("farm");
	EXPECT_EQ(switcher.nextId(), "market");
	switcher.switchTo("market");
	EXPECT_EQ(switcher.nextId(), "bedroom");
	switcher.switchTo("bedroom");
	EXPECT_EQ(switcher.nextId(), "farm");
}

TEST(SceneSwitcherTest, KnowsAnIdIfAnySourceHasIt) {
	const FakeSource editor({"farm"}, 20);
	const FakeSource rooms({"bedroom"}, 10);
	const world::SceneSwitcher switcher({&editor, &rooms}, "bedroom",
					    initialScene());
	EXPECT_TRUE(switcher.knows("farm"));
	EXPECT_TRUE(switcher.knows("bedroom"));
	EXPECT_FALSE(switcher.knows("kitchen"));
}

TEST(SceneSwitcherTest, NextIdWithoutScenesKnownKeepsTheCurrent) {
	const world::SceneSwitcher switcher({}, "bedroom", initialScene());
	EXPECT_EQ(switcher.nextId(), "bedroom");
}

}  // namespace
