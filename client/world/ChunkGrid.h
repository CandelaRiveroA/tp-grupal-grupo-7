#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

#include "world/ScenePixelMapper.h"

namespace world {

// Índice espacial liviano sobre rectángulos de mundo. Guarda índices de una
// colección externa, no punteros ni assets. El consumidor decide qué hacer
// con cada índice; las consultas devuelven índices únicos, ordenados y
// estables respecto al orden de inserción.
class ChunkGrid {
public:
	using Index = std::uint32_t;

	ChunkGrid();
	ChunkGrid(const ChunkGrid&) = delete;
	ChunkGrid& operator=(const ChunkGrid&) = delete;

	// Tamaño del lado del chunk en píxeles. Debe ser positivo.
	void setChunkSize(int chunkSizePixels);

	void clear();
	void insert(Index index, const PixelRect& bounds);
	void query(const PixelRect& visibleWorld, std::vector<Index>& out) const;

private:
	struct ChunkRange {
		ChunkRange();
		bool empty() const;

		int firstX;
		int lastX;
		int firstY;
		int lastY;
	};

	ChunkRange chunkRangeFor(const PixelRect& rect) const;
	std::uint32_t packChunkKey(int chunkX, int chunkY) const;
	int chunkIndex(int pixel) const;

	int chunkSizePixels_;
	std::unordered_map<std::uint32_t, std::vector<Index>> chunks_;
};

}  // namespace world
