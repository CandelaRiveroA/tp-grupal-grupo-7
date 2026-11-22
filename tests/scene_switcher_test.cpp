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

}  // namespace
