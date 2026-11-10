#include "world_model/SerializerIO.h"

#include <fstream>

namespace worldmodel {

void SerializerIO::save(const std::string& text,
			    const std::string& path) {
	std::ofstream stream(path);
	if (!stream.is_open()) {
		throw FormatError(FormatErrorKind::UnwritableOutput, path);
	}
	stream << text;
	stream.close();
	if (!stream) {
		throw FormatError(FormatErrorKind::UnwritableOutput, path);
	}
}

}
