#pragma once

#include <functional>
#include <string>
#include <vector>

namespace world {

// Ancho en px de un texto con la fuente con la que se va a dibujar.
using TextMeasure = std::function<int(const std::string&)>;

// Wrap por palabras. Una palabra que no cabe sola arranca en línea
// nueva y puede exceder maxWidth. El texto
// vacío, o un maxWidth <= 0, dan {""}.
std::vector<std::string> wrapChatText(const std::string& text, int maxWidth,
				      const TextMeasure& measure);

// Borra el último codepoint UTF-8 entero
void eraseLastCodepoint(std::string& text);

// Renglones visibles del historial: mensajes enteros desde el más
// nuevo; el último siempre entra, recortado si él solo excede maxRows.
// Cada mensaje ya viene envuelto en renglones.
std::vector<std::string> visibleHistory(
	const std::vector<std::vector<std::string>>& messages, int maxRows);

}