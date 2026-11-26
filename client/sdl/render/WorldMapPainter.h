#pragma once

#include <SDL2pp/SDL2pp.hh>

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "game/WorldMapState.h"
#include "world/Color.h"
#include "world/PixelBox.h"
#include "world/WorldMapConfig.h"
#include "world/WorldMapLayout.h"

namespace sdl {

class InventoryHud;
class PlayerAppearancePainter;
class SdlContext;
class TextRenderer;

// Dibuja el overlay modal del mapa del mundo usando la configuración y la
// geometría calculada en world::WorldMapConfig / world::WorldMapLayout.
class WorldMapPainter {
public:
    WorldMapPainter(SdlContext& context, InventoryHud& inventoryHud,
                    TextRenderer& text,
                    const PlayerAppearancePainter& appearancePainter,
                    world::WorldMapConfig config,
                    const std::string& minimapDir);

    bool hitsSwitchArrow(int screenW, int screenH, const std::string& exterior,
                         int x, int y) const;

    void draw(const std::string& activeExterior,
              const std::vector<game::WorldMapPin>& pins, int screenW,
              int screenH);

private:
    struct ExteriorEntry {
        std::string title;
        std::optional<SDL2pp::Texture> texture;
    };

    world::WorldMapLayout layoutFor(int screenW, int screenH,
                                    const std::string& exterior) const;
    std::optional<SDL2pp::Texture> tryLoadTexture(const std::string& path) const;
    void drawArrow(std::optional<SDL2pp::Texture>& texture,
                   const std::string& fallbackText,
                   const world::PixelBox& box);
    void drawPin(const game::WorldMapPin& pin,
                 const world::WorldMapLayout& layout) const;
    SDL2pp::Rect toSdlRect(const world::PixelBox& box) const;
    SDL2pp::Color toSdlColor(const world::Color& color) const;

    SdlContext& context_;
    InventoryHud& inventoryHud_;
    TextRenderer& text_;
    const PlayerAppearancePainter& appearancePainter_;
    world::WorldMapConfig config_;

    std::unordered_map<std::string, ExteriorEntry> exteriors_;
    std::optional<SDL2pp::Texture> leftArrowTex_;
    std::optional<SDL2pp::Texture> rightArrowTex_;
};

}  // namespace sdl
