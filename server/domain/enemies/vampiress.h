#ifndef VAMPIRESS_H
#define VAMPIRESS_H

#include "vampire.h"

// Variante rápida de la pareja: más frágil y con menos daño, pero dos
// acciones por tick. Su frenesí duplica la velocidad, no el daño.
class Vampiress: public Vampire {
public:
    Vampiress(uint16_t x, uint16_t y);
    std::string kind_name() const override;
    ~Vampiress() override = default;

    void enter_frenzy() override;
};

#endif  // VAMPIRESS_H
