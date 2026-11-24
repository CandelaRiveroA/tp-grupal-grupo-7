#ifndef COMMUNITY_SET_REWARDS_H
#define COMMUNITY_SET_REWARDS_H

#include <functional>
#include <unordered_map>

#include "community_center.h"

class World;

class CommunitySetRewards {
private:
    std::unordered_map<CommunitySet, std::function<void(World&)>> rewards;

public:
    CommunitySetRewards();

    void apply(World& world, CommunitySet set) const;
};

#endif  // COMMUNITY_SET_REWARDS_H
