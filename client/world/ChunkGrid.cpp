#include "world/ChunkGrid.h"

#include <algorithm>
#include <limits>
#include <stdexcept>

#include "world/WorldConstants.h"

namespace world {

ChunkGrid::ChunkRange::ChunkRange()
	: firstX(0), lastX(-1), firstY(0), lastY(-1) {}

bool ChunkGrid::ChunkRange::empty() const {
	return firstX > lastX || firstY > lastY;
}

ChunkGrid::ChunkGrid() : chunkSizePixels_(kDefaultChunkSizePixels) {}

void ChunkGrid::setChunkSize(const int chunkSizePixels) {
	if (chunkSizePixels <= 0) {
		throw std::invalid_argument("chunk size must be positive");
	}
	if (chunkSizePixels_ != chunkSizePixels) {
		chunkSizePixels_ = chunkSizePixels;
		chunks_.clear();
	}
}

void ChunkGrid::clear() {
	for (auto& entry : chunks_) {
		entry.second.clear();
	}
}

void ChunkGrid::insert(const Index index, const PixelRect& bounds) {
	const ChunkRange range = chunkRangeFor(bounds);
	if (range.empty()) {
		return;
	}
	for (int chunkY = range.firstY; chunkY <= range.lastY; ++chunkY) {
		for (int chunkX = range.firstX; chunkX <= range.lastX;
		     ++chunkX) {
			chunks_[packChunkKey(chunkX, chunkY)].push_back(index);
		}
	}
}

void ChunkGrid::query(const PixelRect& visibleWorld,
		      std::vector<Index>& out) const {
	out.clear();

	const ChunkRange range = chunkRangeFor(visibleWorld);
	if (range.empty()) {
		return;
	}
	for (int chunkY = range.firstY; chunkY <= range.lastY; ++chunkY) {
		for (int chunkX = range.firstX; chunkX <= range.lastX;
		     ++chunkX) {
			const auto entry =
				chunks_.find(packChunkKey(chunkX, chunkY));
			if (entry == chunks_.end()) {
				continue;
			}
			out.insert(out.end(), entry->second.begin(),
				   entry->second.end());
		}
	}

	std::sort(out.begin(), out.end());
	out.erase(std::unique(out.begin(), out.end()), out.end());
}

ChunkGrid::ChunkRange ChunkGrid::chunkRangeFor(const PixelRect& rect) const {
	if (rect.width <= 0 || rect.height <= 0) {
		return ChunkRange();
	}

	ChunkRange range;
	range.firstX = chunkIndex(rect.x);
	range.lastX = chunkIndex(rect.x + rect.width - 1);
	range.firstY = chunkIndex(rect.y);
	range.lastY = chunkIndex(rect.y + rect.height - 1);
	return range;
}

std::uint32_t ChunkGrid::packChunkKey(const int chunkX, const int chunkY) const {
	if (chunkX < std::numeric_limits<std::int16_t>::min() ||
	    chunkX > std::numeric_limits<std::int16_t>::max() ||
	    chunkY < std::numeric_limits<std::int16_t>::min() ||
	    chunkY > std::numeric_limits<std::int16_t>::max()) {
		throw std::out_of_range("chunk coordinate out of range");
	}

	const auto xBits = static_cast<std::uint16_t>(
		static_cast<std::int16_t>(chunkX));
	const auto yBits = static_cast<std::uint16_t>(
		static_cast<std::int16_t>(chunkY));
	return (static_cast<std::uint32_t>(xBits) << 16) |
	       static_cast<std::uint32_t>(yBits);
}

int ChunkGrid::chunkIndex(const int pixel) const {
	if (pixel >= 0) {
		return pixel / chunkSizePixels_;
	}
	return -((-pixel + chunkSizePixels_ - 1) / chunkSizePixels_);
}

}  // namespace world
