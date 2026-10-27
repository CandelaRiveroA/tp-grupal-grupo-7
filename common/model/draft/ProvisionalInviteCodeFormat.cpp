#include "common/model/draft/ProvisionalInviteCodeFormat.h"

#include <string_view>

namespace {
constexpr std::string_view kAllowedChars = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
}

bool ProvisionalInviteCodeFormat::isValid(const std::string& code) const {
	if (code.size() != kLength) {
		return false;
	}
	for (const char c : code) {
		if (kAllowedChars.find(c) == std::string_view::npos) {
			return false;
		}
	}
	return true;
}
