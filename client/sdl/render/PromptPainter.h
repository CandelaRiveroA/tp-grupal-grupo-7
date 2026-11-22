#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "game/PopupState.h"
#include "sdl/render/SpriteSheet.h"
#include "world/InventoryLayout.h"
#include "world/PromptLayout.h"

namespace sdl {
class SdlContext;
class TextRenderer;
}

namespace sdl {

// Pinta el contenido del popup: la pregunta con sus opciones (la que
// está bajo el mouse, en el color hover), el avance "Esperando al
// resto..." con su botón de cancelar, o nada. Solo dibuja lo que
// recibe: la vista y los números del estado, la opción hovereada y la
// caja del panel. La geometría la calcula PromptLayout y las letras las
// dibuja TextRenderer.
class PromptPainter {
public:
	PromptPainter(SdlContext& context, TextRenderer& text,
		      const world::PromptLayout& layout);

	void draw(const game::PopupState& state, int hoveredOption,
		  const world::PixelBox& panel);

private:
	// Cómo se dibuja cada vista, por tabla (§9: sin switch de opciones
	// de dibujo).
	using DrawMethod = void (PromptPainter::*)(const game::PopupState&,
						   int,
						   const world::PixelBox&);

	void drawQuestion(const game::PopupState& state, int hoveredOption,
			  const world::PixelBox& panel);
	void drawWaiting(const game::PopupState& state, int hoveredOption,
			 const world::PixelBox& panel);
	void drawNextDay(const game::PopupState& state, int hoveredOption,
			 const world::PixelBox& panel);
	void drawHidden(const game::PopupState& state, int hoveredOption,
			const world::PixelBox& panel);

	// Un botón del popup (por nombre) en su lugar del panel: las piezas
	// de su 9-slice estiradas a su caja.
	void drawButton(const std::string& name, const world::PixelBox& panel);

	// Reemplaza los tokens ({0}, {1}, ...) por los valores del estado.
	// Es el único lugar que formatea el copy del popup.
	std::string fillTokens(
		const std::string& text,
		const std::vector<std::pair<std::string, std::string>>& tokens)
		const;

	std::map<game::PopupView, DrawMethod> drawMethods_;
	TextRenderer& text_;
	const world::PromptLayout& layout_;
	std::map<std::string, SpriteSheet> buttonSheets_;
};

}  // namespace sdl
