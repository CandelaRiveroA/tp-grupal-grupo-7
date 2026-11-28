#include "world/ContentCatalog.h"

#include "world/CropQualityNames.h"

namespace world {

const SpriteVisual* ContentCatalog::find(const std::string& contentId) const {
	const auto it = visuals.find(contentId);
	return it == visuals.end() ? nullptr : &it->second;
}

const SpriteVisual* ContentCatalog::findQualified(
	const std::string& contentId, const CropQuality quality) const {
	// La tabla de sufijos vive en CropQualityNames (convención §4/§9):
	// una sola instancia por llamada (liviana: 5 entradas), sin switch
	// ni if-else sobre el enum. No static, prohibido por §2.
	const CropQualityNames names;
	if (quality != CropQuality::Base) {
		for (const auto& [name, q] : names.entries()) {
			if (q == quality) {
				const auto it =
					visuals.find(contentId + ":" + name);
				if (it != visuals.end()) {
					return &it->second;
				}
				break;
			}
		}
	}
	// Fallback: contentId plano (Base, o contenido sin calidad).
	return find(contentId);
}

}  // namespace world
