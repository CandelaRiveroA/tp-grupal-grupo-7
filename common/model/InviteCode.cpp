#include "common/model/InviteCode.h"

#include <string_view>

// Vocabulario del formato: 32 símbolos, mayúsculas y dígitos, sin los
// ambiguos (0, O, 1, I) para que el tipeo a mano no se confunda.
namespace {
constexpr std::string_view kCharacterVocabulary = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
}

InviteCode::InviteCode()
	: codeCharacters(kCharacterVocabulary)
	, engine(std::random_device{}())
	, characterIndex(0, codeCharacters.size() - 1) {}

bool InviteCode::isValid(const std::string& code) const {
	if (code.size() != kLength) {
		return false;
	}
	for (const char c : code) {
		if (kCharacterVocabulary.find(c) == std::string_view::npos) {
			return false;
		}
	}
	return true;
}

std::string InviteCode::generate() {
	std::string code;
	code.reserve(kLength);
	for (std::size_t i = 0; i < kLength; ++i) {
		code.push_back(codeCharacters[characterIndex(engine)]);
	}
	return code;
}
