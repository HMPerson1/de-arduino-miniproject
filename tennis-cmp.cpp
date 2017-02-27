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
void runMatch() {
  while (! won(sets_to_win_match, 1, state.pa_match_sets, state.pb_match_sets)) runSet();
  if (state.pa_match_sets > state.pb_match_sets) celebrate(Player::kA);
  else celebrate(Player::kB);
  waitForButton();
  state = State();
}
void runSet() {
  while (! won(6, 2, state.pa_set_games(), state.pb_set_games())) runGame();
  if (state.pa_set_games() > state.pb_set_games()) state.pa_match_sets++;
  else state.pb_match_sets++;
  state.set_number++;
}
void runGame() {
  while (! won(4, 2, state.pa_game_points, state.pb_game_points)) runPoint();
  switch (state.serve) {
  case Player::kA: state.serve = Player::kB; break;
  case Player::kB: state.serve = Player::kA; break;
  }
  if (state.pa_game_points > state.pb_game_points) state.pa_set_games()++;
  else state.pb_set_games()++;
  state.pa_game_points = 0;
  state.pb_game_points = 0;
  if (((state.pa_set_games() + state.pb_set_games()) % 2) != 0) state.position_swapped = !state.position_swapped;
}
void runPoint() {
  showState(state);
  auto point_side = waitForButton();
  state.game_points(state.side_player(point_side))++;
}
constexpr bool won(uint8_t threshold, uint8_t diff, uint8_t score1, uint8_t score2) {
  return ((score1 >= threshold && score1 >= score2 + diff)
          || (score2 >= threshold && score2 >= score1 + diff));
}
