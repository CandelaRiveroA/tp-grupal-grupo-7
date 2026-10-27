#include <cstdint>

#include "client/qt/util/IndexCycle.h"
#include "gtest/gtest.h"

TEST(IndexCycleTest, NextWrapsForward) {
    const IndexCycle cycle(4);

    EXPECT_EQ(cycle.next(0), 1);
    EXPECT_EQ(cycle.next(1), 2);
    EXPECT_EQ(cycle.next(3), 0);
}

TEST(IndexCycleTest, PreviousWrapsBackward) {
    const IndexCycle cycle(4);

    EXPECT_EQ(cycle.previous(0), 3);
    EXPECT_EQ(cycle.previous(2), 1);
    EXPECT_EQ(cycle.previous(3), 2);
}

TEST(IndexCycleTest, SingleOptionDoesNotMove) {
    const IndexCycle cycle(1);

    EXPECT_EQ(cycle.next(0), 0);
    EXPECT_EQ(cycle.previous(0), 0);
}

TEST(IndexCycleTest, ZeroOptionsIsNoOp) {
    const IndexCycle cycle(0);

    EXPECT_EQ(cycle.next(2), 2);
    EXPECT_EQ(cycle.previous(2), 2);
}
