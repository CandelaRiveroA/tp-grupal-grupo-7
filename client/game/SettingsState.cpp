#include "game/SettingsState.h"

#include <algorithm>

#include "world/WorldConstants.h"

namespace game {

SettingsState::SettingsState(const std::map<world::SoundCategory, int>& initialVolumes):
        volumes_(initialVolumes) {}

bool SettingsState::isOpen() const { return open_; }

void SettingsState::toggle() {
    if (open_) {
        close();
        return;
    }
    open_ = true;
}

void SettingsState::close() {
    open_ = false;
    dragging_.reset();
}

bool SettingsState::modal() const { return open_; }

void SettingsState::setVolume(const world::SoundCategory category, const int percent) {
    const auto entry = volumes_.find(category);
    if (entry == volumes_.end()) {
        return;
    }
    entry->second = std::clamp(percent, world::kMinVolumePercent, world::kMaxVolumePercent);
}

int SettingsState::volume(const world::SoundCategory category) const {
    return volumes_.at(category);
}

bool SettingsState::isMuted(const world::SoundCategory category) const {
    return volume(category) == world::kMinVolumePercent;
}

const std::map<world::SoundCategory, int>& SettingsState::volumes() const { return volumes_; }

void SettingsState::beginDrag(const world::SoundCategory category) { dragging_ = category; }

void SettingsState::endDrag() { dragging_.reset(); }

const std::optional<world::SoundCategory>& SettingsState::dragging() const { return dragging_; }

void SettingsState::requestQr() { qrRequested_ = true; }

bool SettingsState::takeQrRequest() {
    const bool requested = qrRequested_;
    qrRequested_ = false;
    return requested;
}

void SettingsState::requestLeave() { leaveRequested_ = true; }

bool SettingsState::takeLeaveRequest() {
    const bool requested = leaveRequested_;
    leaveRequested_ = false;
    return requested;
}

void SettingsState::setQrCode(const std::string& code) { qrCode_ = code; }

const std::string& SettingsState::qrCode() const { return qrCode_; }

}  // namespace game
