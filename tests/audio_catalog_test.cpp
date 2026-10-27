#include "gtest/gtest.h"

#include <string_view>

#include "sdl/audio/AudioCatalog.h"

namespace {

TEST(AudioCatalogTest, CluckIsRegisteredAsSfx) {
	// AudioPlayer carga TODAS las filas Sfx al arrancar: sin la
	// fila, el cacareo nunca suena.
	bool found = false;
	for (const sdl::SoundSpec& spec : sdl::kSounds) {
		found = found ||
			(std::string_view(spec.file) ==
			 "sfx/farm/chicken_cluck.ogg" &&
			 spec.category == sdl::SoundCategory::Sfx);
	}
	EXPECT_TRUE(found);
}

}
