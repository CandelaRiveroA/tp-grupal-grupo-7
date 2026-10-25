#pragma once

#include <cstddef>
#include <string>

#include "common/model/draft/InviteCodeFormat.h"

// 6 caracteres: letras mayúsculas y dígitos, sin 0, O, 1 ni I.
class ProvisionalInviteCodeFormat : public InviteCodeFormat {
public:
	static constexpr std::size_t kLength = 6;

	bool isValid(const std::string& code) const override;
};
