#include "core/Constants.h"

namespace core {

std::string assetPath(const std::string& file) {
	return std::string(kAssetsDir) + "/" + file;
}

std::string soundPath(const std::string& file) {
	return std::string(kSoundsDir) + "/" + file;
}

}
