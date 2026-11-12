#pragma once

#include <array>
#include <cstddef>
#include <map>
#include <string>
#include <vector>

#include "toml/TomlDocument.h"
#include "world/ActionNames.h"
#include "world/CharacterAppearance.h"
#include "world/CharacterCatalog.h"
#include "world/CreatureCatalog.h"

namespace world {

// Traduce characters.toml a un CharacterCatalog (datos puros).
// Mismo patrón que InventoryTomlReader / ToolOverlayTomlReader: clase
// propia, métodos de instancia, claves como constantes (en el .cpp),
// sin funciones libres ni helpers static.
class CharacterTomlReader {
public:
	explicit CharacterTomlReader(const tomlcfg::TomlDocument& doc);

	CharacterCatalog readCatalog() const;

private:
	std::string join(const std::string& base,
			 const std::string& key) const;

	std::string idx(const std::string& base, std::size_t i) const;

	LayerFrame readLayer(const std::string& posePath,
			     std::size_t layerIdx) const;

	Pose readPose(const std::string& posePath) const;

	std::vector<Pose> readFrames(const std::string& framesPath) const;

	ActionData readAction(const std::string& actionPath,
			      int defaultFrameTicks) const;

	std::map<ActionKind, ActionData> readActions(
		const std::string& dirBase, int defaultFrameTicks) const;

	DirectionAppearance readDirection(const std::string& dirBase,
					  int defaultFrameTicks) const;

	CharacterCatalog::CreatureEntry readCreature(
		const std::string& base) const;

	MoveCadence readCadence(const std::string& base) const;

	// Presentación del contenido sostenido (subtabla opcional `hold`;
	// sin ella quedan los defaults: pegado a la cabeza, escala 1).
	HoldPresentation readHold(const std::string& base) const;

	// Apariencia personalizable del jugador (tabla opcional `appearance`).
	// Sin tabla devuelve `present = false` (animales).
	AppearanceData readAppearance(const std::string& base) const;

	SkinAppearanceData readSkinAppearance(const std::string& base) const;

	AppearanceLayerData readAppearanceLayer(const std::string& base) const;

	AppearanceOption readAppearanceOption(
		const std::string& optionPath) const;

	// Array plano [col,row,cols,rows, ...] -> lista de regiones.
	std::vector<SheetRegion> readAppearanceRegions(
		const std::string& path) const;

	std::vector<int> readInts(const std::string& path) const;

	std::array<DirectionAppearance, 4> readDirections(
		const std::string& base, const MoveCadence& cadence) const;

	void requirePositive(int value, const std::string& path) const;

	void requireNonNegative(int value, const std::string& path) const;

	const tomlcfg::TomlDocument& doc_;
	// Vocabulario compartido de nombres de acción (ver ActionNames).
	ActionNames actionNames_;
};

}
