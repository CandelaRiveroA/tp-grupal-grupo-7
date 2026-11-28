#include "sdl/render/WorldMapPainter.h"

#include <exception>
#include <filesystem>
#include <utility>

#include "core/Constants.h"
#include "sdl/context/SdlContext.h"
#include "sdl/render/InventoryHud.h"
#include "sdl/render/PlayerAppearancePainter.h"
#include "sdl/render/TextRenderer.h"

namespace sdl {

WorldMapPainter::WorldMapPainter(SdlContext& context, InventoryHud& inventoryHud,
                                 TextRenderer& text,
                                 const PlayerAppearancePainter& appearancePainter,
                                 world::WorldMapConfig config,
                                 const std::string& minimapDir)
    : context_(context)
    , inventoryHud_(inventoryHud)
    , text_(text)
    , appearancePainter_(appearancePainter)
    , config_(std::move(config)) {
    for (const world::WorldMapExteriorConfig& ext : config_.exteriors) {
        std::optional<SDL2pp::Texture> texture;
        if (!minimapDir.empty()) {
            texture = tryLoadTexture(
                    (std::filesystem::path(minimapDir) / (ext.id + ".png")).string());
        }
        exteriors_.emplace(ext.id, ExteriorEntry{ext.title, std::move(texture)});
    }
    leftArrowTex_ = tryLoadTexture(core::assetPath(config_.arrowLeftSheet));
    rightArrowTex_ = tryLoadTexture(core::assetPath(config_.arrowRightSheet));
}

bool WorldMapPainter::hitsSwitchArrow(const int screenW, const int screenH,
                                      const std::string& exterior,
                                      const int x, const int y) const {
    return layoutFor(screenW, screenH, exterior).hitsSwitchArrow(x, y);
}

world::WorldMapLayout WorldMapPainter::layoutFor(const int screenW, const int screenH,
                                                 const std::string& exterior) const {
    world::PixelSize mapSize{0, 0};
    const auto it = exteriors_.find(exterior);
    if (it != exteriors_.end() && it->second.texture.has_value()) {
        mapSize = world::PixelSize{it->second.texture->GetWidth(),
                                   it->second.texture->GetHeight()};
    }
    return world::WorldMapLayout(screenW, screenH, config_, mapSize);
}

void WorldMapPainter::draw(const std::string& activeExterior,
                           const std::vector<game::WorldMapPin>& pins,
                           const int screenW, const int screenH) {
    context_.fillRect(SDL2pp::Rect(0, 0, screenW, screenH),
                      toSdlColor(config_.backdropColor));

    const world::WorldMapLayout layout = layoutFor(screenW, screenH, activeExterior);

    // drawPanel(0, 0) dibuja el marco de madera sin grilla de slots; el fondo
    // de pergamino se rellena explícitamente debajo.
    context_.fillRect(toSdlRect(layout.backgroundBox()),
                      toSdlColor(config_.parchmentColor));
    inventoryHud_.drawPanel(layout.panel(), 0, 0);

    const auto it = exteriors_.find(activeExterior);
    const std::string title =
            (it != exteriors_.end()) ? it->second.title : activeExterior;
    text_.draw(title, layout.titleBox(), config_.inkColor, config_.titleFontSize, true);

    if (it != exteriors_.end() && it->second.texture.has_value()) {
        SDL2pp::Texture& tex = *it->second.texture;
        context_.fillRect(toSdlRect(layout.mapBorderBox()),
                          toSdlColor(config_.woodBorderColor));
        context_.drawTexture(
                tex, SDL2pp::Rect(0, 0, tex.GetWidth(), tex.GetHeight()),
                toSdlRect(layout.mapRect()));
    } else {
        text_.draw(config_.unavailableText, layout.mapRect(), config_.inkColor,
                   config_.unavailableFontSize);
    }

    drawArrow(leftArrowTex_, config_.arrowLeftFallback, layout.leftArrowRect());
    drawArrow(rightArrowTex_, config_.arrowRightFallback, layout.rightArrowRect());

    // Compañeros primero (cabezas y luego nombres para que una cabeza no
    // tape otro nombre); el jugador local al final para que su aro dorado
    // y su nombre queden siempre visibles.
    drawPins(pins, layout, false);
    drawPins(pins, layout, true);
}

std::optional<SDL2pp::Texture> WorldMapPainter::tryLoadTexture(
        const std::string& path) const {
    if (path.empty()) {
        return std::nullopt;
    }
    try {
        return context_.createTexture(path);
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

void WorldMapPainter::drawArrow(std::optional<SDL2pp::Texture>& texture,
                                const std::string& fallbackText,
                                const world::PixelBox& box) {
    if (texture.has_value()) {
        context_.drawTexture(
                *texture,
                SDL2pp::Rect(0, 0, texture->GetWidth(), texture->GetHeight()),
                toSdlRect(box));
        return;
    }
    text_.draw(fallbackText, box, config_.inkColor, config_.arrowFontSize, true);
}

void WorldMapPainter::drawPins(const std::vector<game::WorldMapPin>& pins,
                               const world::WorldMapLayout& layout,
                               const bool localOnly) {
    for (const game::WorldMapPin& pin : pins) {
        if (pin.isLocal == localOnly) {
            drawPinHead(pin, layout);
        }
    }
    for (const game::WorldMapPin& pin : pins) {
        if (pin.isLocal == localOnly) {
            drawPinLabel(pin, layout);
        }
    }
}

void WorldMapPainter::drawPinHead(const game::WorldMapPin& pin,
                                  const world::WorldMapLayout& layout) const {
    const world::PixelBox outerBox = layout.pinOuterBox(pin.normX, pin.normY);
    const world::PixelBox innerBox = layout.pinInnerBox(outerBox);

    const SDL2pp::Color ringColor = toSdlColor(
            pin.isLocal ? config_.localRingColor : config_.woodBorderColor);
    context_.fillRect(toSdlRect(outerBox), ringColor);
    context_.fillRect(toSdlRect(innerBox), toSdlColor(config_.parchmentColor));
    appearancePainter_.drawHead(pin.appearance, toSdlRect(innerBox));
}

void WorldMapPainter::drawPinLabel(const game::WorldMapPin& pin,
                                   const world::WorldMapLayout& layout) {
    if (pin.displayName.empty()) {
        return;
    }
    const world::PixelBox outerBox = layout.pinOuterBox(pin.normX, pin.normY);
    const int textWidth = text_.textWidth(pin.displayName, config_.pinLabel.fontSize,
                                          config_.pinLabel.bold, config_.pinLabel.font);
    const world::PixelBox box = layout.pinLabelBox(outerBox, textWidth);
    context_.fillRect(toSdlRect(box), toSdlColor(config_.parchmentColor));
    text_.draw(pin.displayName, box, config_.inkColor, config_.pinLabel.fontSize,
               config_.pinLabel.bold, config_.pinLabel.font);
}

SDL2pp::Rect WorldMapPainter::toSdlRect(const world::PixelBox& box) const {
    return SDL2pp::Rect(box.x, box.y, box.width, box.height);
}

SDL2pp::Color WorldMapPainter::toSdlColor(const world::Color& color) const {
    return SDL2pp::Color(static_cast<Uint8>(color.r),
                         static_cast<Uint8>(color.g),
                         static_cast<Uint8>(color.b),
                         static_cast<Uint8>(color.a));
}

}  // namespace sdl
