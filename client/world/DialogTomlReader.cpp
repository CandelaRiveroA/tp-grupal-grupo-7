#include "world/DialogTomlReader.h"

#include <array>

#include "world/ConfigError.h"
#include "world/SpriteVisual.h"

namespace world {

namespace {

using tomlcfg::TomlDocument;

// Claves del TOML.
constexpr const char* kAnimation = "animation";
constexpr const char* kFrames = "frames";
constexpr const char* kFrameTicks = "frame_ticks";
constexpr const char* kLoops = "loops";
constexpr const char* kContent = "content";
constexpr const char* kAnchorOffset = "anchor_offset";
constexpr const char* kContentOffset = "offset";
constexpr const char* kAt = "at";

// Indicación -> tabla del TOML que la describe. Sumar una indicación =
// una fila acá + su entrada en dialogs.toml; el dibujo no cambia.
struct IndicationBinding {
	Indication kind;
	const char* tableKey;
};

constexpr std::array<IndicationBinding, 2> kIndicationBindings = {{
	{Indication::ReadyToMilk, "ready_to_milk"},
	{Indication::ReadyToWater, "ready_to_water"},
}};

}

DialogTomlReader::DialogTomlReader(const TomlDocument& doc): doc_(doc) {}

std::string DialogTomlReader::join(const std::string& base,
				   const std::string& key) const {
	return base + "." + key;
}

std::string DialogTomlReader::idx(const std::string& base,
				  std::size_t i) const {
	return base + "[" + std::to_string(i) + "]";
}

DialogAnimation DialogTomlReader::readAnimation(
	const std::string& base) const {
	DialogAnimation animation;

	// Hoja/celda/tamaño de los frames con el parser compartido (la
	// celda de cada frame está en `frames`, no en `at`).
	animation.sprite = SpriteVisual(doc_, base);

	const std::string framesPath = join(base, kFrames);
	const std::size_t count = doc_.arraySize(framesPath);
	if (count == 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue, framesPath);
	}
	for (std::size_t i = 0; i < count; ++i) {
		animation.frames.push_back(
			SheetCell(doc_, idx(framesPath, i)));
	}

	const std::string ticksPath = join(base, kFrameTicks);
	animation.frameTicks = doc_.getInt(ticksPath);
	if (animation.frameTicks <= 0) {
		throw ConfigError(ConfigErrorKind::InvalidValue, ticksPath);
	}

	const std::string loopsPath = join(base, kLoops);
	animation.loops = doc_.hasKey(loopsPath) && doc_.getBool(loopsPath);
	return animation;
}

DialogData DialogTomlReader::readDialog(const std::string& base) const {
	DialogData data;
	data.animation = readAnimation(join(base, kAnimation));
	data.anchorOffset = readOffset(join(base, kAnchorOffset));

	// Contenido adentro: OPCIONAL (hay diálogos cuyo elemento ya
	// viene en el último frame de la animación). Con contenido, su
	// corrección también es opcional (default: centrado).
	const std::string contentPath = join(base, kContent);
	if (doc_.hasKey(contentPath)) {
		data.content = SpriteVisual(doc_, contentPath, kAt);
		const std::string contentOffsetPath =
			join(contentPath, kContentOffset);
		if (doc_.hasKey(contentOffsetPath)) {
			data.contentOffset = readOffset(contentOffsetPath);
		}
	}
	return data;
}

PixelPoint DialogTomlReader::readOffset(const std::string& key) const {
	// Offset en px: puede ser negativo (el diálogo va sobre la entidad).
	const std::array<int, 2> offset = doc_.getIntArray<2>(key);
	return PixelPoint{offset[0], offset[1]};
}

DialogCatalog DialogTomlReader::read() const {
	DialogCatalog catalog;
	for (const IndicationBinding& binding : kIndicationBindings) {
		catalog.dialogs.emplace(binding.kind,
					readDialog(binding.tableKey));
	}
	return catalog;
}

}
