#include "world/ChatWrap.h"

namespace world {

namespace {

// UTF-8: los bytes de continuación (rol "10xxxxxx") son 0x80..0xBF;
// ninguno arranca un codepoint.
constexpr unsigned char kContinuationFirst = 0x80;
constexpr unsigned char kContinuationLast = 0xBF;

bool isContinuation(unsigned char c) {
	return c >= kContinuationFirst && c <= kContinuationLast;
}

std::vector<std::string> wordsOf(const std::string& text) {
	// Cualquier espacio separa: el dictado crudo puede traer '\n'.
	std::vector<std::string> words;
	std::string word;
	for (const char c: text) {
		if (c == ' ' || c == '\n' || c == '\t') {
			if (!word.empty()) {
				words.push_back(word);
			}
			word.clear();
		} else {
			word += c;
		}
	}
	if (!word.empty()) {
		words.push_back(word);
	}
	return words;
}

}  // namespace

std::vector<std::string> wrapChatText(const std::string& text, int maxWidth,
				      const TextMeasure& measure) {
	if (maxWidth <= 0) {
		return {""};
	}

	std::vector<std::string> lines;
	std::string line;
	for (const std::string& word: wordsOf(text)) {
		std::string candidate = line.empty() ? word : line + " " + word;
		if (measure(candidate) <= maxWidth) {
			line = candidate;
			continue;
		}
		if (!line.empty()) {
			lines.push_back(line);
			line.clear();
		}
		// Una palabra que no cabe sola arranca en línea nueva; si es
		// más ancha que la caja desborda el panel.
		// shortcut: sin partida silábica ni corte por codepoints.
		line = word;
	}
	lines.push_back(line);
	return lines;
}

void eraseLastCodepoint(std::string& text) {
	while (!text.empty() && isContinuation(text.back())) {
		text.pop_back();
	}
	if (!text.empty()) {
		text.pop_back();
	}
}

std::vector<std::string> visibleHistory(
	const std::vector<std::vector<std::string>>& messages, const int maxRows) {
	std::vector<std::string> lines;
	if (messages.empty() || maxRows <= 0) {
		return lines;
	}

	// Desde el más nuevo hacia atrás: un mensaje entra si sus renglones
	// caben enteros.
	std::size_t first = messages.size() - 1;
	int rows = static_cast<int>(messages[first].size());
	while (first > 0) {
		const int candidate = static_cast<int>(messages[first - 1].size());
		if (rows + candidate > maxRows) {
			break;
		}
		rows += candidate;
		--first;
	}

	// El tope de acá solo puede recortar al último mensaje.
	for (std::size_t i = first; i < messages.size(); ++i) {
		for (const std::string& line: messages[i]) {
			if (static_cast<int>(lines.size()) >= maxRows) {
				return lines;
			}
			lines.push_back(line);
		}
	}
	return lines;
}

}