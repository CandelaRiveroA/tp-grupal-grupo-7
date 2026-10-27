#pragma once

namespace game {

// Encapsula el slot activo del inventario. Escala a cualquier cantidad
// de slots sin cambiar código (slotCount es parámetro de select()).
class InventorySelector {
public:
	static constexpr int kDefaultSlot = 0;

	InventorySelector();

	// Cambia el slot activo; ignora si slot está fuera de [0, slotCount).
	void select(int slot, int slotCount);

	int active() const;

private:
	int activeSlot_;
};

}
