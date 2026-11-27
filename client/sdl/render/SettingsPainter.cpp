#include "sdl/render/SettingsPainter.h"

#include <cstddef>
#include <string>

#include "core/Constants.h"
#include "sdl/context/SdlContext.h"
#include "sdl/render/TextRenderer.h"
#include "world/DigitCatalog.h"
#include "world/WorldConstants.h"

namespace sdl {

SettingsPainter::SettingsPainter(SdlContext& context, TextRenderer& text,
				 ButtonPainter& buttons, DigitPainter& digits,
				 const world::SettingsLayout& layout)
	: context_(context)
	, text_(text)
	, buttons_(buttons)
	, digits_(digits)
	, layout_(layout)
	, tabSheet_(context, core::assetPath(layout.tab.sheet),
		    layout.tab.cell.width, layout.tab.cell.height)
	, knobSheet_(context, core::assetPath(layout.knobImage),
		     layout.slider.knob.width, layout.slider.knob.height)
	, iconSheet_(context, core::assetPath(layout.icon.image),
		     layout.icon.cell.width, layout.icon.cell.height)
	, mutedIconSheet_(context, core::assetPath(layout.icon.mutedImage),
			  layout.icon.cell.width, layout.icon.cell.height) {}

void SettingsPainter::draw(const game::SettingsState& state,
			   const world::PixelBox& panel) {
	const world::SettingsBoxes boxes = layout_.boxes(panel);
	const world::Color& ink = layout_.textColor;

	tabSheet_.draw(layout_.tab.at.col, layout_.tab.at.row,
		       SDL2pp::Rect(boxes.tab.x, boxes.tab.y, boxes.tab.width,
				    boxes.tab.height));
	text_.draw(layout_.title, boxes.title, ink, layout_.titleSize);
	drawMultiplayer(state, boxes);
	text_.drawLeft(layout_.audioTitle, boxes.audioTitle, ink,
		       layout_.subtitleSize);
	for (std::size_t i = 0; i < boxes.volume.size(); ++i) {
		drawVolumeRow(state, layout_.volumeRows[i], boxes.volume[i]);
	}
	drawLeave(boxes);
}

void SettingsPainter::drawLeave(const world::SettingsBoxes& boxes) {
	text_.drawLeft(layout_.leaveLabel, boxes.leaveLabel, layout_.textColor,
		       layout_.labelSize);
	buttons_.draw(world::kPromptCancelButton, boxes.leaveButton);
}

void SettingsPainter::drawMultiplayer(const game::SettingsState& state,
				      const world::SettingsBoxes& boxes) {
	const world::Color& ink = layout_.textColor;
	text_.drawLeft(layout_.multiplayerTitle, boxes.multiplayerTitle, ink,
		       layout_.subtitleSize);
	text_.drawLeft(layout_.qrLabel, boxes.qrLabel, ink, layout_.labelSize);
	// El espacio reservado: vacío hasta que el servidor mande el código.
	if (!state.qrCode().empty()) {
		text_.drawLeft(state.qrCode(), boxes.qrArea, ink,
			       layout_.labelSize);
	}
	buttons_.draw(world::kPromptOkButton, boxes.qrButton);
}

void SettingsPainter::drawVolumeRow(const game::SettingsState& state,
				    const world::VolumeRow& row,
				    const world::VolumeRowBoxes& boxes) {
	const world::Color& ink = layout_.textColor;
	const int volume = state.volume(row.category);

	text_.drawLeft(row.label, boxes.label, ink, layout_.labelSize);

	const world::PixelBox line = layout_.slider.lineBox(boxes.track);
	const world::Color& lineColor = layout_.slider.lineColor;
	context_.fillRect(
		SDL2pp::Rect(line.x, line.y, line.width, line.height),
		SDL2pp::Color(static_cast<Uint8>(lineColor.r),
			      static_cast<Uint8>(lineColor.g),
			      static_cast<Uint8>(lineColor.b),
			      static_cast<Uint8>(lineColor.a)));
	const world::PixelBox knob = layout_.slider.knobBox(boxes.track, volume);
	knobSheet_.draw(0, 0,
			SDL2pp::Rect(knob.x, knob.y, knob.width, knob.height));

	// Con volumen 0 el ícono pasa al bloqueado.
	SpriteSheet& icon = state.isMuted(row.category) ? mutedIconSheet_
							: iconSheet_;
	icon.draw(0, 0,
		  SDL2pp::Rect(boxes.icon.x, boxes.icon.y, boxes.icon.width,
			       boxes.icon.height));

	const std::vector<int> digits = world::digitsOf(volume);
	const world::PercentBoxes percent = layout_.percentBoxes(
		boxes.percent, static_cast<int>(digits.size()));
	digits_.draw(digits, percent.digits);
	text_.drawLeft(layout_.percent.sign, percent.sign, ink,
		       layout_.labelSize);
}

}  // namespace sdl
