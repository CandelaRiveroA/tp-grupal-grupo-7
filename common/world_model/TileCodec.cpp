#include "world_model/TileCodec.h"

#include <charconv>
#include <system_error>

#include "world_model/FormatError.h"

namespace worldmodel {

std::optional<TileRef> TileCodec::decode(const std::string& text,
					 const std::string& path) {
	if (text.empty()) {
		return std::nullopt;
	}
	const std::size_t colon = text.find(':');
	const std::size_t comma = text.find(',');
	const bool malformed =
		colon == std::string::npos || comma == std::string::npos ||
		colon == 0 || comma < colon ||
		text.find(':', colon + 1) != std::string::npos ||
		text.find(',', comma + 1) != std::string::npos;
	int column = 0;
	int row = 0;
	if (malformed ||
	    !parseInt(text.substr(colon + 1, comma - colon - 1), column) ||
	    !parseInt(text.substr(comma + 1), row)) {
		throw FormatError(FormatErrorKind::InvalidValue, path);
	}
	return TileRef{text.substr(0, colon), column, row};
}

std::string TileCodec::encode(const std::optional<TileRef>& tile) {
	return tile ? tile->key() : std::string();
}

bool TileCodec::parseInt(const std::string& text, int& value) {
	const char* begin = text.data();
	const char* end = begin + text.size();
	const std::from_chars_result result = std::from_chars(begin, end, value);
	return result.ec == std::errc() && result.ptr == end && value >= 0;
}

}
