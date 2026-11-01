#include "matches_monitor.h"

#include <memory>
#include <string>

#include "gtest/gtest.h"

#include "common_match_request.h"
#include "server_output_monitor.h"

namespace {

std::shared_ptr<Queue<ServerMessage>> statuses() {
    return std::make_shared<Queue<ServerMessage>>();
}

}  // namespace

TEST(MatchesMonitorTest, CreatorAndJoinersFillMatchUpToMax) {
    ServerOutputMonitor output;
    MatchesMonitor matches(output);

    const MatchSeat creator = matches.create("alfa", statuses());
    EXPECT_EQ(creator.status, LobbyStatus::Ok);
    EXPECT_EQ(creator.player_id, 1);

    for (uint8_t expected = 2; expected <= MAX_JUGADORES; ++expected) {
        const MatchSeat seat = matches.join("alfa", statuses());
        EXPECT_EQ(seat.status, LobbyStatus::Ok);
        EXPECT_EQ(seat.player_id, expected);
    }

    EXPECT_EQ(matches.join("alfa", statuses()).status, LobbyStatus::Full);
    matches.closeAll();
}

TEST(MatchesMonitorTest, RejectsDuplicateUnknownAndEmptyNames) {
    ServerOutputMonitor output;
    MatchesMonitor matches(output);

    EXPECT_EQ(matches.create("alfa", statuses()).status, LobbyStatus::Ok);
    EXPECT_EQ(matches.create("alfa", statuses()).status, LobbyStatus::NameTaken);
    EXPECT_EQ(matches.join("beta", statuses()).status, LobbyStatus::NotFound);
    EXPECT_EQ(matches.create("", statuses()).status, LobbyStatus::Invalid);
    EXPECT_EQ(matches.join("", statuses()).status, LobbyStatus::Invalid);
    matches.closeAll();
}

TEST(MatchesMonitorTest, NameIsFreedWhenLastPlayerLeaves) {
    ServerOutputMonitor output;
    MatchesMonitor matches(output);

    const MatchSeat first = matches.create("alfa", statuses());
    const MatchSeat second = matches.join("alfa", statuses());
    matches.leave("alfa", first.player_id);
    EXPECT_EQ(matches.create("alfa", statuses()).status, LobbyStatus::NameTaken);

    matches.leave("alfa", second.player_id);
    EXPECT_EQ(matches.join("alfa", statuses()).status, LobbyStatus::NotFound);
    EXPECT_EQ(matches.create("alfa", statuses()).status, LobbyStatus::Ok);
    matches.closeAll();
}

TEST(MatchesMonitorTest, MatchesAreIndependent) {
    ServerOutputMonitor output;
    MatchesMonitor matches(output);

    EXPECT_EQ(matches.create("alfa", statuses()).status, LobbyStatus::Ok);
    EXPECT_EQ(matches.create("beta", statuses()).status, LobbyStatus::Ok);

    const MatchSeat seat = matches.join("beta", statuses());
    EXPECT_EQ(seat.status, LobbyStatus::Ok);
    EXPECT_EQ(seat.player_id, 2);
    matches.closeAll();
}
