#ifndef COMMUNITY_SET_CATALOG_H
#define COMMUNITY_SET_CATALOG_H

#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

#include "community_center.h"

class CommunitySetCatalog {
private:
    std::unordered_map<std::string, CommunitySet> sets_by_name;
    std::unordered_map<uint8_t, std::string> names_by_id;

public:
    CommunitySetCatalog();

    CommunitySet set_for_name(const std::string& name) const;
    const std::string& name_for_id(uint8_t id) const;
    std::vector<std::string> all_names() const;
};

#endif  // COMMUNITY_SET_CATALOG_H
