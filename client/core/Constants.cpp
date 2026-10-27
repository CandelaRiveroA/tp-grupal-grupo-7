#include "core/Constants.h"

namespace core {

std::string assetPath(const std::string& file) {
	return kAssetsDir + "/" + file;
}

std::string soundPath(const std::string& file) {
	return kSoundsDir + "/" + file;
}

}
