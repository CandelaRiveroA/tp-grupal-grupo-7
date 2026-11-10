#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <vector>

#include "gtest/gtest.h"
#include "world/ChunkGrid.h"

namespace {

using world::ChunkGrid;
using world::PixelRect;

TEST(ChunkGridTest, ReturnsOnlyIndexesWhoseChunksIntersectTheView) {
	ChunkGrid grid;
	grid.insert(0, PixelRect{0, 0, 10, 10});
	grid.insert(1, PixelRect{260, 0, 10, 10});

	std::vector<std::uint32_t> visible;
	grid.query(PixelRect{0, 0, 100, 100}, visible);

	ASSERT_EQ(visible.size(), 1u);
	EXPECT_EQ(visible[0], 0u);
}

TEST(ChunkGridTest, DeduplicatesIndexesStoredInSeveralChunks) {
	ChunkGrid grid;
	grid.insert(2, PixelRect{0, 0, 600, 10});

	std::vector<std::uint32_t> visible;
	grid.query(PixelRect{0, 0, 600, 10}, visible);

	ASSERT_EQ(visible.size(), 1u);
	EXPECT_EQ(visible[0], 2u);
}

TEST(ChunkGridTest, ReturnsIndexesSortedByInsertionIndex) {
	ChunkGrid grid;
	grid.insert(4, PixelRect{0, 0, 10, 10});
	grid.insert(1, PixelRect{0, 0, 10, 10});
	grid.insert(3, PixelRect{0, 0, 10, 10});

	std::vector<std::uint32_t> visible;
	grid.query(PixelRect{0, 0, 10, 10}, visible);

	ASSERT_EQ(visible.size(), 3u);
	EXPECT_EQ(visible[0], 1u);
	EXPECT_EQ(visible[1], 3u);
	EXPECT_EQ(visible[2], 4u);
}

TEST(ChunkGridTest, ClearRemovesPreviousIndexes) {
	ChunkGrid grid;
	grid.insert(7, PixelRect{0, 0, 10, 10});
	grid.clear();

	std::vector<std::uint32_t> visible;
	grid.query(PixelRect{0, 0, 10, 10}, visible);

	EXPECT_TRUE(visible.empty());
}

TEST(ChunkGridTest, HandlesNegativeCoordinates) {
	ChunkGrid grid;
	grid.insert(1, PixelRect{-5, -5, 10, 10});

	std::vector<std::uint32_t> visible;
	grid.query(PixelRect{-5, -5, 10, 10}, visible);

	ASSERT_EQ(visible.size(), 1u);
	EXPECT_EQ(visible[0], 1u);
}

TEST(ChunkGridTest, ChunkSizeCanBeConfigured) {
	ChunkGrid grid;
	grid.setChunkSize(64);
	grid.insert(3, PixelRect{60, 0, 10, 10});
	grid.insert(4, PixelRect{70, 0, 10, 10});

	std::vector<std::uint32_t> visible;
	grid.query(PixelRect{60, 0, 4, 10}, visible);

	ASSERT_EQ(visible.size(), 1u);
	EXPECT_EQ(visible[0], 3u);
}

TEST(ChunkGridTest, RejectsNonPositiveChunkSize) {
	ChunkGrid grid;
	EXPECT_THROW(grid.setChunkSize(0), std::invalid_argument);
	EXPECT_THROW(grid.setChunkSize(-1), std::invalid_argument);
}

}  // namespace
