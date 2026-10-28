#include "gtest/gtest.h"

#include <string_view>

#include "sdl/audio/AudioCatalog.h"

namespace {

bool isSfx(std::string_view file) {
	for (const sdl::SoundSpec& spec : sdl::kSounds) {
		if (std::string_view(spec.file) == file &&
		    spec.category == sdl::SoundCategory::Sfx) {
			return true;
		}
	}
	return false;
}

TEST(AudioCatalogTest, CluckIsRegisteredAsSfx) {
	// AudioPlayer carga TODAS las filas Sfx al arrancar: sin la
	// fila, el cacareo nunca suena.
	EXPECT_TRUE(isSfx("sfx/farm/chicken_cluck.ogg"));
}

TEST(AudioCatalogTest, CowMooIsRegisteredAsSfx) {
	EXPECT_TRUE(isSfx("sfx/farm/cow.ogg"));
}

}
