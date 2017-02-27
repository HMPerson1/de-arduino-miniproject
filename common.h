#ifndef COMMON_H
#define COMMON_H

#include <inttypes.h>

namespace config {
  constexpr uint8_t kSetsPerMatch = 3;
  static_assert(kSetsPerMatch > 0 && kSetsPerMatch % 2 != 0);
}

enum class Player {
  kA,
  kB,
};

enum class CourtSide {
  kWest,
  kEast,
};

struct State {
  Player serve;
  bool position_swapped;
  uint8_t pa_game_points;
  uint8_t pb_game_points;
  uint8_t set_number;
  uint8_t pa_match_sets_games[config::kSetsPerMatch];
  uint8_t pb_match_sets_games[config::kSetsPerMatch];
  uint8_t pa_match_sets;
  uint8_t pb_match_sets;

  inline uint8_t& pa_set_games() { return pa_match_sets_games[set_number]; }
  inline uint8_t& pb_set_games() { return pb_match_sets_games[set_number]; }
  inline constexpr Player west_player() { return side_player(CourtSide::kWest); }
  inline constexpr Player east_player() { return side_player(CourtSide::kEast); }
  inline constexpr Player side_player(CourtSide s) { return position_swapped ^ (s == CourtSide::kWest) ? Player::kA : Player::kB; }
  inline constexpr CourtSide serve_side() { return position_swapped ^ (serve == Player::kA) ? CourtSide::kWest: CourtSide::kEast; }
  inline uint8_t& game_points(Player p) { return p == Player::kA ? pa_game_points : pb_game_points; }
  inline uint8_t& west_game_points() { return game_points(west_player()); }
  inline uint8_t& east_game_points() { return game_points(east_player()); }
};

#endif /* COMMON_H */

