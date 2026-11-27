#include "matches_monitor.h"

#include <algorithm>
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

    const MatchSeat creator = matches.create("alfa", "ad", "juan", "ana", statuses());
    EXPECT_EQ(creator.status, LobbyStatus::Ok);
    EXPECT_EQ(creator.player_id, 1);

    // ad.toml tiene 1 cabaña -> capacidad 1: el segundo join es Full.
    EXPECT_EQ(matches.join("alfa", "jugador2", "personaje2", statuses()).status, LobbyStatus::Full);
    matches.closeAll();
}

TEST(MatchesMonitorTest, CatalogoIncluyeMundosValidadosPorElServidor) {
    ServerOutputMonitor output;
    MatchesMonitor matches(output);

    const std::vector<std::string> worlds = matches.validWorldNames();

    EXPECT_NE(std::find(worlds.begin(), worlds.end(), "ad"), worlds.end());
}

TEST(MatchesMonitorTest, JoinDevuelveOkConSegundoJugadorEnMundoDeDosCabinas) {
    ServerOutputMonitor output;
    MatchesMonitor matches(output);

    const MatchSeat creator = matches.create("alfa", "ad2", "juan", "ana", statuses());
    EXPECT_EQ(creator.status, LobbyStatus::Ok);
    EXPECT_EQ(creator.player_id, 1);

    const MatchSeat joiner = matches.join("alfa", "pedro", "bea", statuses());
    EXPECT_EQ(joiner.status, LobbyStatus::Ok);
    EXPECT_EQ(joiner.player_id, 2);

    // Tercero ya no cabe (capacidad = 2).
    EXPECT_EQ(matches.join("alfa", "extra", "otro", statuses()).status, LobbyStatus::Full);
    matches.closeAll();
}

TEST(MatchesMonitorTest, RejectsDuplicateUnknownAndEmptyNames) {
    ServerOutputMonitor output;
    MatchesMonitor matches(output);

    EXPECT_EQ(matches.create("alfa", "ad", "juan", "ana", statuses()).status, LobbyStatus::Ok);
    EXPECT_EQ(matches.create("alfa", "ad", "pedro", "bea", statuses()).status, LobbyStatus::NameTaken);
    EXPECT_EQ(matches.join("beta", "pedro", "bea", statuses()).status, LobbyStatus::NotFound);
    EXPECT_EQ(matches.create("", "ad", "pedro", "bea", statuses()).status, LobbyStatus::Invalid);
    EXPECT_EQ(matches.join("", "pedro", "bea", statuses()).status, LobbyStatus::Invalid);
    matches.closeAll();
}

TEST(MatchesMonitorTest, NameIsFreedWhenLastPlayerLeaves) {
    ServerOutputMonitor output;
    MatchesMonitor matches(output);

    const MatchSeat first = matches.create("alfa", "ad", "juan", "ana", statuses());
    EXPECT_EQ(first.status, LobbyStatus::Ok);
    matches.leave("alfa", first.player_id);
    EXPECT_EQ(matches.join("alfa", "pedro", "bea", statuses()).status, LobbyStatus::NotFound);
    EXPECT_EQ(matches.create("alfa", "ad", "juan", "ana", statuses()).status, LobbyStatus::Ok);
    matches.closeAll();
}

TEST(MatchesMonitorTest, MatchesAreIndependent) {
    ServerOutputMonitor output;
    MatchesMonitor matches(output);

    EXPECT_EQ(matches.create("alfa", "ad", "juan", "ana", statuses()).status, LobbyStatus::Ok);
    EXPECT_EQ(matches.create("beta", "ad", "pedro", "bea", statuses()).status, LobbyStatus::Ok);
    matches.closeAll();
}

TEST(MatchesMonitorTest, RejectsEmptyAndTooLongUsernames) {
    ServerOutputMonitor output;
    MatchesMonitor matches(output);

    EXPECT_EQ(matches.create("alfa", "ad", "", "ana", statuses()).status, LobbyStatus::Invalid);
    EXPECT_EQ(matches.create("alfa", "ad", std::string(kMaxUsernameLength + 1, 'a'), "ana", statuses())
                      .status,
              LobbyStatus::Invalid);
    EXPECT_EQ(matches.create("alfa", "ad", std::string(kMaxUsernameLength, 'a'), "ana", statuses())
                      .status,
              LobbyStatus::Ok);
    matches.closeAll();
}

TEST(MatchesMonitorTest, RejectsEmptyAndTooLongCharacterNames) {
    ServerOutputMonitor output;
    MatchesMonitor matches(output);

    EXPECT_EQ(matches.create("alfa", "ad", "juan", "", statuses()).status, LobbyStatus::Invalid);
    EXPECT_EQ(matches.create("alfa", "ad", "juan", std::string(kMaxCharacterNameLength + 1, 'a'),
                             statuses()).status,
              LobbyStatus::Invalid);
    EXPECT_EQ(matches.create("alfa", "ad", "juan", std::string(kMaxCharacterNameLength, 'a'), statuses())
                      .status,
              LobbyStatus::Ok);
    matches.closeAll();
}

TEST(MatchesMonitorTest, SameUsernameCannotJoinTwoMatches) {
    ServerOutputMonitor output;
    MatchesMonitor matches(output);

    EXPECT_EQ(matches.create("alfa", "ad", "juan", "ana", statuses()).status, LobbyStatus::Ok);
    EXPECT_EQ(matches.create("beta", "ad", "juan", "bea", statuses()).status, LobbyStatus::UsernameTaken);
    EXPECT_EQ(matches.join("alfa", "juan", "bea", statuses()).status, LobbyStatus::UsernameTaken);
    matches.closeAll();
}

TEST(MatchesMonitorTest, UsernameIsFreedWhenPlayerLeaves) {
    ServerOutputMonitor output;
    MatchesMonitor matches(output);

    const MatchSeat seat = matches.create("alfa", "ad", "juan", "ana", statuses());
    matches.leave("alfa", seat.player_id);
    EXPECT_EQ(matches.create("beta", "ad", "juan", "ana", statuses()).status, LobbyStatus::Ok);
    matches.closeAll();
}

TEST(MatchesMonitorTest, SameCharacterCannotJoinSameMatch) {
    ServerOutputMonitor output;
    MatchesMonitor matches(output);

    EXPECT_EQ(matches.create("alfa", "ad", "juan", "ana", statuses()).status, LobbyStatus::Ok);
    EXPECT_EQ(matches.join("alfa", "pedro", "ana", statuses()).status,
              LobbyStatus::CharacterNameTaken);
    matches.closeAll();
}

TEST(MatchesMonitorTest, SameCharacterIsAllowedInDifferentMatches) {
    ServerOutputMonitor output;
    MatchesMonitor matches(output);

    EXPECT_EQ(matches.create("alfa", "ad", "juan", "ana", statuses()).status, LobbyStatus::Ok);
    EXPECT_EQ(matches.create("beta", "ad", "pedro", "ana", statuses()).status, LobbyStatus::Ok);
    matches.closeAll();
}
