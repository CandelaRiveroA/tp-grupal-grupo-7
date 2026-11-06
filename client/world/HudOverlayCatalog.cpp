#include "world/HudOverlayCatalog.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

namespace world {

namespace {

constexpr int kChannelMax = 255;
constexpr char kTokenOpen = '{';
constexpr char kTokenClose = '}';
constexpr char kPadChar = '0';

}  // namespace

const std::vector<HudOverlay>& HudOverlayCatalog::overlays() const {
	return overlays_;
}

void HudOverlayCatalog::add(const HudOverlay& overlay) {
	overlays_.push_back(overlay);
}

int HudOverlayCatalog::alphaFor(const HudOverlay& overlay,
				const double intensity) const {
	const double clamped = std::clamp(intensity, 0.0, 1.0);
	return static_cast<int>(std::lround(overlay.alpha * clamped));
}

Color HudOverlayCatalog::multiplyColorFor(const HudOverlay& overlay,
					  const double intensity) const {
	// Fuerza del efecto (0..1): alpha del overlay por la intensidad.
	const double strength =
		static_cast<double>(alphaFor(overlay, intensity)) / kChannelMax;
	const Color target = overlay.color.value_or(
		Color{kChannelMax, kChannelMax, kChannelMax, kChannelMax});
	const auto mix = [strength](const int channel) {
		return static_cast<int>(std::lround(
			kChannelMax + (channel - kChannelMax) * strength));
	};
	return Color{mix(target.r), mix(target.g), mix(target.b), kChannelMax};
}

const std::vector<HudLabel>& HudOverlayCatalog::labels() const {
	return labels_;
}

void HudOverlayCatalog::addLabel(const HudLabel& label) {
	labels_.push_back(label);
}

std::string HudOverlayCatalog::textFor(
	const HudLabel& label, const std::map<std::string, int>& values) const {
	// Recorre la plantilla copiando lo literal y reemplazando cada
	// {token} por su texto. Un token sin texto queda tal cual.
	std::string result;
	std::size_t at = 0;
	while (at < label.text.size()) {
		const std::size_t open = label.text.find(kTokenOpen, at);
		const std::size_t close =
			open == std::string::npos
				? std::string::npos
				: label.text.find(kTokenClose, open);
		if (close == std::string::npos) {
			result += label.text.substr(at);
			break;
		}
		result += label.text.substr(at, open - at);
		const std::string token =
			label.text.substr(open + 1, close - open - 1);
		const std::optional<std::string> text =
			tokenText(label, token, values);
		result += text.has_value()
				  ? *text
				  : label.text.substr(open, close - open + 1);
		at = close + 1;
	}
	return result;
}

std::optional<std::string> HudOverlayCatalog::tokenText(
	const HudLabel& label, const std::string& token,
	const std::map<std::string, int>& values) const {
	// Sin formato declarado, el token es su propio valor, tal cual.
	const auto format = label.tokens.find(token);
	const HudTokenFormat plain;
	const HudTokenFormat& rule =
		format == label.tokens.end() ? plain : format->second;
	const std::string& key = rule.source.empty() ? token : rule.source;

	const auto raw = values.find(key);
	if (raw == values.end()) {
		return std::nullopt;
	}
	int value = raw->second / rule.divisor;
	if (rule.modulo > 0) {
		value %= rule.modulo;
	}
	if (!rule.names.empty()) {
		if (value < 0 ||
		    value >= static_cast<int>(rule.names.size())) {
			return std::nullopt;
		}
		return rule.names[static_cast<std::size_t>(value)];
	}
	std::string digits = std::to_string(value);
	if (static_cast<int>(digits.size()) < rule.pad) {
		digits.insert(0, static_cast<std::size_t>(rule.pad) -
					 digits.size(),
			      kPadChar);
	}
	return digits;
}

}
