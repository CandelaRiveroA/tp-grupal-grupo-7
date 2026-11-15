#pragma once

#include <string>

#include "world_model/ValidationIssue.h"

namespace worldmodel {

// Valida un ValidationIssue.
class IssueFormatter {
public:
	std::string format(const ValidationIssue& issue) const;
};

}  // namespace worldmodel
