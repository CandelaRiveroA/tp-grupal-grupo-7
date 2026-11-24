#include "world/SliderLayout.h"

#include <algorithm>

#include "world/WorldConstants.h"

namespace world {

PixelBox SliderLayout::lineBox(const PixelBox& track) const {
    return PixelBox{track.x + knob.width / 2, track.y + (track.height - lineThickness) / 2,
                    track.width - knob.width, lineThickness};
}

PixelBox SliderLayout::knobBox(const PixelBox& track, const int percent) const {
    const int clamped = std::clamp(percent, kMinVolumePercent, kMaxVolumePercent);
    const int travel = track.width - knob.width;
    return PixelBox{track.x + clamped * travel / kMaxVolumePercent,
                    track.y + (track.height - knob.height) / 2, knob.width, knob.height};
}

int SliderLayout::percentAt(const PixelBox& track, const int x) const {
    const int travel = track.width - knob.width;
    if (travel <= 0) {
        return kMinVolumePercent;
    }
    const int offset = std::clamp(x - track.x - knob.width / 2, 0, travel);
    return (offset * kMaxVolumePercent + travel / 2) / travel;
}

}  // namespace world
