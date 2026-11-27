#ifndef ANIMAL_PURCHASE_H
#define ANIMAL_PURCHASE_H

enum class AnimalPurchaseResult {
    Success,
    InvalidQuantity,
    UnknownAnimal,
    LimitReached,
    NoSpace,
    InsufficientGold,
    BarnUnavailable,
};

#endif  // ANIMAL_PURCHASE_H
