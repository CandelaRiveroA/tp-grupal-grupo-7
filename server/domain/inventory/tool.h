#ifndef TOOL_H
#define TOOL_H

#include "item.h"
#include <memory>

class Tool : public Item {
public:
    explicit Tool(ItemID id);
    ~Tool() override = default;

    std::unique_ptr<Item> clone() const override;
};

#endif // TOOL_H
