#pragma once

namespace core {

// Archivos de datos del campo (cultivos, terreno y lugar de plantado),
// relativos al directorio de assets. Aparte de Constants.h para que
// sumar el campo no toque las constantes de la sesión.
constexpr const char* kCropsFile = "crops.toml";

constexpr const char* kGroundFile = "ground.toml";

constexpr const char* kBuildPlaceFile = "buildplace.toml";

}  // namespace core
