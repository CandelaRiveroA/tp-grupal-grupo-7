#ifndef MARNIE_H
#define MARNIE_H

#include <cstdint>
#include <string>
#include <map>

#include "../animals/animal_purchase.h"
#include "merchant.h"

class Marnie: public Merchant {
private:
    std::map<AnimalPurchaseResult, std::string> purchase_errors;

protected:
    std::string handle_buy(Player& p, const std::string& item_name, uint16_t qty,
                           World& w) override;
    std::string handle_sell(Player& p, const std::string& item_name, uint16_t qty,
                            World& w) override;

public:
    Marnie(uint16_t x, uint16_t y);
    ~Marnie() override = default;
};

#endif  // MARNIE_H
