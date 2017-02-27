static Adafruit_PCD8544 display = Adafruit_PCD8544(7, 5, 6);
void displaySetup() {
  pinMode(9, OUTPUT);
  digitalWrite(9, HIGH);
  display.begin();
  displayInit();
  display.setTextWrap(false);
}
void displayInit() {
  SPI.setClockDivider(SPI_CLOCK_DIV8);
  display.command(0x21);
  display.command(0xC0);
  display.command(0x14);
  display.command(0x20);
  display.command(0x0C);
  display.display();
}
void showSplash() {
  display.clearDisplay();
  display.setFont(&FreeSans9pt7b);
  printCentered("TENNIS", 29);
  display.setFont(NULL);
  display.display();
}
void showWhereStart() {
  display.clearDisplay();
  printCentered("Where does", 12);
  printCentered("player A", 20);
  printCentered("start?", 28);
  display.setCursor(0, 24); display.print('<');
  display.setCursor(79, 24); display.print('>');
  display.display();
}
void showWhoServe(State state) {
  display.clearDisplay();
  printCentered("Who serves", 16);
  printCentered("first?", 24);
  display.setCursor(0, 24);
  display.print('<');
  display.print(p2c(state.west_player()));
  display.setCursor(73, 24);
  display.print(p2c(state.east_player()));
  display.print('>');
  display.display();
}
void showState(State state) {
  static_assert(config::kSetsPerMatch == 3);
  display.clearDisplay();
  display.setCursor(2, 1); display.print("Set");
  display.setCursor(38, 1);
  switch (state.set_number) {
  case 0:
    display.fillRect(37, 0, 7, 9, BLACK);
    display.setTextColor(WHITE);
    display.print('1');
    display.setTextColor(BLACK);
    display.print("  2  3");
    break;
  case 1:
    display.fillRect(55, 0, 7, 9, BLACK);
    display.print("1  ");
    display.setTextColor(WHITE);
    display.print('2');
    display.setTextColor(BLACK);
    display.print("  3");
    break;
  case 2:
    display.fillRect(73, 0, 7, 9, BLACK);
    display.print("1  2  ");
    display.setTextColor(WHITE);
    display.print('3');
    display.setTextColor(BLACK);
    break;
  }
  display.drawFastHLine(0, 10, 84, BLACK);
  display.drawChar(2, 13, 'A', BLACK, WHITE, 1);
  display.drawChar(2, 21, 'B', BLACK, WHITE, 1);
  display.setCursor(35, 13);
  for (auto a_games : state.pa_match_sets_games) { printInt2(a_games); display.print(' '); }
  display.setCursor(35, 21);
  for (auto b_games : state.pb_match_sets_games) { printInt2(b_games); display.print(' '); }
  display.drawFastHLine(0, 30, 84, BLACK);
  display.drawFastHLine(0, 32, 84, BLACK);
  display.drawChar(2, 37, p2c(state.west_player()), BLACK, WHITE, 1);
  display.drawChar(77, 37, p2c(state.east_player()), BLACK, WHITE, 1);
  display.drawRect((state.serve_side() == CourtSide::kWest ? 0 : 75), 35, 9, 11, BLACK);
  display.drawFastHLine(39, 40, 6, BLACK);
  if (state.pa_game_points < 3 || state.pb_game_points < 3) {
    if (state.west_game_points() == 0) { display.setCursor(15, 37); display.print("Love"); }
    else { display.setCursor(27, 37); printPointsNormal(state.west_game_points()); }
    display.setCursor(46, 37);
    if (state.east_game_points() == 0) display.print("Love");
    else printPointsNormal(state.east_game_points());
  } else {
    switch (state.west_game_points() - state.east_game_points()) {
    case 0:
      display.drawFastHLine(39, 40, 6, WHITE);
      display.setCursor(27, 37); display.print("DEUCE");
      break;
    case -1:
      display.setCursor(27, 37); display.print(40);
      display.setCursor(46, 37); display.print("Adv");
      break;
    case 1:
      display.setCursor(21, 37); display.print("Adv");
      display.setCursor(46, 37); display.print(40);
      break;
    }
  }
  display.display();
}
void printInt2(uint8_t i) { if (i < 10) display.print('0'); display.print(i); }
void printPointsNormal(uint8_t p) {
  switch (p) {
  case 1: display.print(15); break;
  case 2: display.print(30); break;
  case 3: display.print(40); break;
  default: abort(); break;
  }
}
inline constexpr char p2c(Player p) { return p == Player::kA ? 'A' : 'B'; }
void celebrate(Player player) {
  display.clearDisplay();
  printCentered(player == Player::kA ? "PLAYER A" : "PLAYER B", 16);
  printCentered("WON THE MATCH!", 24);
  display.display();
}
void printCentered(const char s[], int16_t base) {
  int16_t x, y; uint16_t w, h;
  display.getTextBounds((char*)s, 0, 0, &x, &y, &w, &h);
  display.setCursor((display.width() - (int16_t)w) / 2 - x, base);
  display.print(s);
}
