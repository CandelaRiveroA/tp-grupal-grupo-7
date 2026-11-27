#include "animal_catalog.h"

#include "chicken.h"
#include "cow.h"

AnimalCatalog::AnimalCatalog():
        entries{
                {ItemID::COW, {"vacas", "cow", 5, 80, [](uint16_t x, uint16_t y) {
                                   return std::make_unique<Cow>(x, y);
                               }}},
                {ItemID::CHICKEN, {"gallinas", "chicken", 10, 20, [](uint16_t x, uint16_t y) {
                                       return std::make_unique<Chicken>(x, y);
                                   }}},
        } {}

const std::map<ItemID, AnimalOffer>& AnimalCatalog::offers() const { return entries; }

const AnimalOffer* AnimalCatalog::find(ItemID type) const {
    const auto found = entries.find(type);
    return found != entries.end() ? &found->second : nullptr;
}
