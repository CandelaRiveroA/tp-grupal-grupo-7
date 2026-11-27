#include "community_set_catalog.h"

#include <stdexcept>

CommunitySetCatalog::CommunitySetCatalog():
        sets_by_name{
                {"cultivos", CommunitySet::CROPS},
                {"minerales", CommunitySet::MINERALS},
                {"metales", CommunitySet::MINERALS},
                {"orbes", CommunitySet::MINERALS},
                {"animales", CommunitySet::ANIMALS},
                {"pelea", CommunitySet::COMBAT},
                {"dinero", CommunitySet::MONEY}},
        names_by_id{
                {1, "cultivos"},
                {2, "minerales"},
                {3, "animales"},
                {4, "pelea"},
                {5, "dinero"}} {}

CommunitySet CommunitySetCatalog::set_for_name(const std::string& name) const {
    const auto it = sets_by_name.find(name);
    return it != sets_by_name.end() ? it->second : static_cast<CommunitySet>(0);
}

const std::string& CommunitySetCatalog::name_for_id(uint8_t id) const {
    const auto it = names_by_id.find(id);
    if (it == names_by_id.end()) {
        throw std::out_of_range("Id de set comunitario desconocido");
    }
    return it->second;
}

std::vector<std::string> CommunitySetCatalog::all_names() const {
    std::vector<std::string> names;
    names.reserve(names_by_id.size());
    for (uint8_t id = 1; id <= 5; ++id) {
        names.push_back(name_for_id(id));
    }
    return names;
}
