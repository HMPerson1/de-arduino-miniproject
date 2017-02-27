#include "tennis.h"

#include <inttypes.h>

#include "common.h"
#include "button.h"
#include "display.h"

enum class SetType {
  kAdvantage,
  kTieBreak,
};

static void runMatch();
static void runSet();
static void runGame();
static void runPoint();
constexpr static bool won(uint8_t threshold, uint8_t diff, uint8_t score1, uint8_t score2);

static State state = State();

void tennisMain() {
  showWhereStart();
  CourtSide a_start = waitForButton();
  state.position_swapped = (a_start == CourtSide::kEast);

  showWhoServe(state);
  CourtSide serve = waitForButton();
  state.serve = state.side_player(serve);

  runMatch();
}

constexpr static int sets_to_win_match = config::kSetsPerMatch / 2 + 1;
static_assert(sets_to_win_match * 2 - 1 == config::kSetsPerMatch);
// first to `sets_to_win_match` sets wins the match
void runMatch() {
  // wait for someone to win
  while (! won(sets_to_win_match, 1, state.pa_match_sets, state.pb_match_sets)) {
    runSet();
  }
  // someone won a match!
  // *** CELEBRATE!!! ***
  if (state.pa_match_sets > state.pb_match_sets) {
    celebrate(Player::kA);
  } else {
    celebrate(Player::kB);
  }
  waitForButton();
  state = State();
}

// first to 6 games wins the set
// service swaps every game
// must win by 2 games
void runSet() {
  // wait for someone to win
  while (! won(6, 2, state.pa_set_games(), state.pb_set_games())) {
    runGame();
  }
  // someone won a set!
  // update score
  if (state.pa_set_games() > state.pb_set_games()) {
    state.pa_match_sets++;
  } else {
    state.pb_match_sets++;
  }
  // update set number
  state.set_number++;
}

// first to 4 points wins the match
// must win by 2 points
void runGame() {
  // wait for someone to win
  while (! won(4, 2, state.pa_game_points, state.pb_game_points)) {
    runPoint();
  }
  // someone won a game!
  // swap service
  switch (state.serve) {
  case Player::kA:
    state.serve = Player::kB;
    break;
  case Player::kB:
    state.serve = Player::kA;
    break;
  }
  // update score
  if (state.pa_game_points > state.pb_game_points) {
    state.pa_set_games()++;
  } else {
    state.pb_set_games()++;
  }
  state.pa_game_points = 0;
  state.pb_game_points = 0;
  // switch sides on odd-numbered games
  if (((state.pa_set_games() + state.pb_set_games()) % 2) != 0) {
    state.position_swapped = !state.position_swapped;
  }
}

void runPoint() {
  // update display
  showState(state);
  // wait for someone to win
  auto point_side = waitForButton();
  // someone won a point!
  // update score
  state.game_points(state.side_player(point_side))++;
}

constexpr bool won(uint8_t threshold, uint8_t diff, uint8_t score1, uint8_t score2) {
  return ((score1 >= threshold && score1 >= score2 + diff)
          || (score2 >= threshold && score2 >= score1 + diff));
}

