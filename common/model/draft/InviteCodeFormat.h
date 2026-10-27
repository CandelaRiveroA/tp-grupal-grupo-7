#pragma once

#include <string>


// Valida solo el formato; que el código exista lo dice el servidor.
class InviteCodeFormat {
public:
	virtual ~InviteCodeFormat() = default;

	virtual bool isValid(const std::string& code) const = 0;
};
