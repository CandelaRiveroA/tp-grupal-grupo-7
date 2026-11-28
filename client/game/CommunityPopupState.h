#pragma once

#include <cstddef>

namespace game {

// Estado de presentación del tablón paginado del Centro Comunitario: si está
// abierto, qué página está seleccionada y si el mini-popup de ayuda está
// abierto.
class CommunityPopupState {
public:
	void open();
	void close();
	bool isOpen() const;

	void openHelp();
	void closeHelp();
	bool isHelpOpen() const;

	std::size_t pageIndex(std::size_t pageCount) const;
	void nextPage(std::size_t pageCount);
	void prevPage(std::size_t pageCount);

private:
	bool open_ = false;
	bool helpOpen_ = false;
	std::size_t pageIndex_ = 0;
};

}  // namespace game
