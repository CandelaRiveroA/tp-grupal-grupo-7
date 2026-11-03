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

    const MatchSeat creator = matches.create("alfa", "juan", "ana", statuses());
    EXPECT_EQ(creator.status, LobbyStatus::Ok);
    EXPECT_EQ(creator.player_id, 1);

    for (uint8_t expected = 2; expected <= MAX_JUGADORES; ++expected) {
        const MatchSeat seat = matches.join("alfa", "jugador" + std::to_string(expected),
                                            "personaje" + std::to_string(expected), statuses());
        EXPECT_EQ(seat.status, LobbyStatus::Ok);
        EXPECT_EQ(seat.player_id, expected);
    }

    EXPECT_EQ(matches.join("alfa", "extra", "otro", statuses()).status, LobbyStatus::Full);
    matches.closeAll();
}

TEST(MatchesMonitorTest, RejectsDuplicateUnknownAndEmptyNames) {
    ServerOutputMonitor output;
    MatchesMonitor matches(output);

    EXPECT_EQ(matches.create("alfa", "juan", "ana", statuses()).status, LobbyStatus::Ok);
    EXPECT_EQ(matches.create("alfa", "pedro", "bea", statuses()).status, LobbyStatus::NameTaken);
    EXPECT_EQ(matches.join("beta", "pedro", "bea", statuses()).status, LobbyStatus::NotFound);
    EXPECT_EQ(matches.create("", "pedro", "bea", statuses()).status, LobbyStatus::Invalid);
    EXPECT_EQ(matches.join("", "pedro", "bea", statuses()).status, LobbyStatus::Invalid);
    matches.closeAll();
}

TEST(MatchesMonitorTest, NameIsFreedWhenLastPlayerLeaves) {
    ServerOutputMonitor output;
    MatchesMonitor matches(output);

    const MatchSeat first = matches.create("alfa", "juan", "ana", statuses());
    const MatchSeat second = matches.join("alfa", "pedro", "bea", statuses());
    matches.leave("alfa", first.player_id);
    EXPECT_EQ(matches.create("alfa", "juan", "ana", statuses()).status, LobbyStatus::NameTaken);

    matches.leave("alfa", second.player_id);
    EXPECT_EQ(matches.join("alfa", "pedro", "bea", statuses()).status, LobbyStatus::NotFound);
    EXPECT_EQ(matches.create("alfa", "juan", "ana", statuses()).status, LobbyStatus::Ok);
    matches.closeAll();
}

TEST(MatchesMonitorTest, MatchesAreIndependent) {
    ServerOutputMonitor output;
    MatchesMonitor matches(output);

    EXPECT_EQ(matches.create("alfa", "juan", "ana", statuses()).status, LobbyStatus::Ok);
    EXPECT_EQ(matches.create("beta", "pedro", "bea", statuses()).status, LobbyStatus::Ok);

    const MatchSeat seat = matches.join("beta", "maria", "carla", statuses());
    EXPECT_EQ(seat.status, LobbyStatus::Ok);
    EXPECT_EQ(seat.player_id, 2);
    matches.closeAll();
}

TEST(MatchesMonitorTest, RejectsEmptyAndTooLongUsernames) {
    ServerOutputMonitor output;
    MatchesMonitor matches(output);

    EXPECT_EQ(matches.create("alfa", "", "ana", statuses()).status, LobbyStatus::Invalid);
    EXPECT_EQ(matches.create("alfa", std::string(kMaxUsernameLength + 1, 'a'), "ana", statuses())
                      .status,
              LobbyStatus::Invalid);
    EXPECT_EQ(matches.create("alfa", std::string(kMaxUsernameLength, 'a'), "ana", statuses())
                      .status,
              LobbyStatus::Ok);
    matches.closeAll();
}

TEST(MatchesMonitorTest, RejectsEmptyAndTooLongCharacterNames) {
    ServerOutputMonitor output;
    MatchesMonitor matches(output);

    EXPECT_EQ(matches.create("alfa", "juan", "", statuses()).status, LobbyStatus::Invalid);
    EXPECT_EQ(matches.create("alfa", "juan", std::string(kMaxCharacterNameLength + 1, 'a'),
                             statuses())
                      .status,
              LobbyStatus::Invalid);
    EXPECT_EQ(matches.create("alfa", "juan", std::string(kMaxCharacterNameLength, 'a'), statuses())
                      .status,
              LobbyStatus::Ok);
    matches.closeAll();
}

TEST(MatchesMonitorTest, SameUsernameCannotJoinTwoMatches) {
    ServerOutputMonitor output;
    MatchesMonitor matches(output);

    EXPECT_EQ(matches.create("alfa", "juan", "ana", statuses()).status, LobbyStatus::Ok);
    EXPECT_EQ(matches.create("beta", "juan", "bea", statuses()).status, LobbyStatus::UsernameTaken);
    EXPECT_EQ(matches.join("alfa", "juan", "bea", statuses()).status, LobbyStatus::UsernameTaken);
    matches.closeAll();
}

TEST(MatchesMonitorTest, UsernameIsFreedWhenPlayerLeaves) {
    ServerOutputMonitor output;
    MatchesMonitor matches(output);

    const MatchSeat seat = matches.create("alfa", "juan", "ana", statuses());
    matches.leave("alfa", seat.player_id);
    EXPECT_EQ(matches.create("beta", "juan", "ana", statuses()).status, LobbyStatus::Ok);
    matches.closeAll();
}

TEST(MatchesMonitorTest, SameCharacterCannotJoinSameMatch) {
    ServerOutputMonitor output;
    MatchesMonitor matches(output);

    EXPECT_EQ(matches.create("alfa", "juan", "ana", statuses()).status, LobbyStatus::Ok);
    EXPECT_EQ(matches.join("alfa", "pedro", "ana", statuses()).status,
              LobbyStatus::CharacterNameTaken);

    const MatchSeat seat = matches.join("alfa", "pedro", "bea", statuses());
    EXPECT_EQ(seat.status, LobbyStatus::Ok);
    EXPECT_EQ(seat.player_id, 2);
    matches.closeAll();
}

TEST(MatchesMonitorTest, SameCharacterIsAllowedInDifferentMatches) {
    ServerOutputMonitor output;
    MatchesMonitor matches(output);

    EXPECT_EQ(matches.create("alfa", "juan", "ana", statuses()).status, LobbyStatus::Ok);
    EXPECT_EQ(matches.create("beta", "pedro", "ana", statuses()).status, LobbyStatus::Ok);
    matches.closeAll();
}
