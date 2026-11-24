#pragma once

#include "game/SettingsState.h"
#include "sdl/render/ButtonPainter.h"
#include "sdl/render/DigitPainter.h"
#include "sdl/render/SpriteSheet.h"
#include "world/PixelBox.h"
#include "world/SettingsLayout.h"

namespace sdl {
class SdlContext;
class TextRenderer;
}

namespace sdl {

// Pinta el contenido del popup de configuración sobre el panel que
// dibuja InventoryHud: pestaña, título, la sección multiplayer (con el
// botón del QR) y una fila de volumen por categoría. Solo dibuja lo que
// recibe: los valores salen de SettingsState y las cajas de
// SettingsLayout (las mismas que usa el click).
class SettingsPainter {
public:
	SettingsPainter(SdlContext& context, TextRenderer& text,
			ButtonPainter& buttons, DigitPainter& digits,
			const world::SettingsLayout& layout);

	void draw(const game::SettingsState& state, const world::PixelBox& panel);

private:
	void drawMultiplayer(const game::SettingsState& state,
			     const world::SettingsBoxes& boxes);
	void drawLeave(const world::SettingsBoxes& boxes);
	void drawVolumeRow(const game::SettingsState& state,
			   const world::VolumeRow& row,
			   const world::VolumeRowBoxes& boxes);

	SdlContext& context_;
	TextRenderer& text_;
	ButtonPainter& buttons_;
	DigitPainter& digits_;
	const world::SettingsLayout& layout_;
	SpriteSheet tabSheet_;
	SpriteSheet knobSheet_;
	SpriteSheet iconSheet_;
	SpriteSheet mutedIconSheet_;
};

}  // namespace sdl
