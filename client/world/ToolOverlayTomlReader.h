#pragma once

#include <cstddef>
#include <map>
#include <string>

#include "toml/TomlDocument.h"
#include "world/ActionNames.h"
#include "world/CharacterAppearance.h"
#include "world/ContentIds.h"
#include "world/ItemKind.h"
#include "world/ToolOverlay.h"

namespace world {

// Traduce tools_overlay.toml a un ToolOverlayCatalog (datos puros).
// Mismo patrón que InventoryTomlReader: clase propia, métodos de
// instancia, claves como constantes (en el .cpp), sin funciones libres
// ni helpers static.
class ToolOverlayTomlReader {
public:
	explicit ToolOverlayTomlReader(const tomlcfg::TomlDocument& doc);

	ToolOverlayCatalog read() const;

private:
	std::string join(const std::string& base,
			 const std::string& key) const;

	std::string idx(const std::string& base, std::size_t i) const;

	std::map<std::string, FrameAnchor> anchorNames() const;

	std::map<FrameAnchor, int> readAnchorOffsets() const;

	OverlayLayer readLayer(const tomlcfg::TomlDocument& layerDoc,
			       const std::string& layerPath) const;

	FrameAnchor readAnchor(const tomlcfg::TomlDocument& frameDoc) const;

	// Ajustes por frame compartidos por herramientas y contenido en
	// manos (ancla, offsets, ángulo, pivote, espejado, escala).
	void readPlacement(const tomlcfg::TomlDocument& frameDoc,
			   OverlayFrame& frame) const;

	ToolOverlayFrame readFrame(const tomlcfg::TomlDocument& frameDoc) const;

	// Campos de animación comunes (hoja, celda, ritmo, acción, frames)
	// que comparten la herramienta y su efecto (el chorro). Lee en
	// `overlay` desde la tabla `base`.
	void readAnimated(const std::string& base,
			  AnimatedOverlay& overlay) const;

	ToolOverlayAction readAction(const std::string& base) const;

	ToolOverlayAppearance readAppearance(const std::string& itemKey) const;

	HeldOverlayFrame readHeldFrame(
		const tomlcfg::TomlDocument& frameDoc) const;

	HeldContentOverlay readHeldContent(const std::string& base) const;

	const tomlcfg::TomlDocument& doc_;
	// Vocabulario compartido de nombres de acción (ver ActionNames).
	ActionNames actionNames_;
	// Vocabulario contentId: los nombres de tabla del TOML (ContentIds).
	ContentIds contentIds_;
};

}
