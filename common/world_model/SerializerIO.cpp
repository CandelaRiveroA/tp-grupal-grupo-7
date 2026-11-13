#include "world_model/SerializerIO.h"

#include <algorithm>
#include <filesystem>
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

std::vector<std::string> SerializerIO::tomlFilesIn(
	const std::string& directory) {
	std::vector<std::string> files;
	try {
		for (const auto& entry :
		     std::filesystem::directory_iterator(directory)) {
			if (entry.path().extension() == ".toml") {
				files.push_back(entry.path().string());
			}
		}
	} catch (const std::filesystem::filesystem_error&) {
		throw FormatError(FormatErrorKind::UnreadableInput, directory);
	}
	std::sort(files.begin(), files.end());
	return files;
}

}
