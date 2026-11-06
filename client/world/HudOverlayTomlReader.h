#pragma once

#include <map>
#include <string>

#include "world/AnchorLayout.h"
#include "world/HudOverlayCatalog.h"

namespace tomlcfg {
class TomlDocument;
}

namespace world {

// Lee ui/hud.toml hacia un HudOverlayCatalog. Mismo patrón que
// DigitTomlReader / CursorTomlReader: TomlDocument -> datos puros.
// Lanza ConfigError si un overlay es inválido.
class HudOverlayTomlReader {
public:
	explicit HudOverlayTomlReader(const tomlcfg::TomlDocument& doc);

	HudOverlayCatalog read() const;

private:
	HudOverlay readOverlay(const tomlcfg::TomlDocument& overlayDoc) const;

	// Una etiqueta de texto [[label]] y el formato de cada uno de sus
	// tokens [label.token.<nombre>].
	HudLabel readLabel(const tomlcfg::TomlDocument& labelDoc) const;
	HudTokenFormat readToken(const tomlcfg::TomlDocument& tokenDoc) const;

	SheetRegion readRegion(const tomlcfg::TomlDocument& overlayDoc) const;

	// Opacidad máxima opcional (`alpha`, 0..255) y color plano de un velo
	// (`color = [r, g, b]`).
	void readAlpha(const tomlcfg::TomlDocument& overlayDoc,
		       HudOverlay& overlay) const;
	Color readColor(const tomlcfg::TomlDocument& overlayDoc) const;

	// Vocabulario blend (string TOML) -> BlendMode. Tabla, no switch (§4).
	std::map<std::string, BlendMode> blendNames() const;
	BlendMode readBlend(const tomlcfg::TomlDocument& overlayDoc) const;

	// Anchor nombrado en el TOML (vocabulario de AnchorLayout::fromName).
	Anchor readAnchor(const tomlcfg::TomlDocument& overlayDoc) const;

	const tomlcfg::TomlDocument& doc_;
};

}
