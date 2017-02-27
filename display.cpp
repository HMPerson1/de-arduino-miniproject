#include "display.h"

#include <inttypes.h>

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <Fonts/FreeSans9pt7b.h>

#include "common.h"

// EVIL HACK
// extern uint8_t pcd8544_buffer[504];

static Adafruit_PCD8544 display = Adafruit_PCD8544(7, 5, 6);

static void displayInit();
static void printInt2(uint8_t i);
static void printPointsNormal(uint8_t p);
static void printCentered(const char s[], int16_t base);
static constexpr char p2c(Player p);
// static void debugDumpDisplayBuffer();

/**
 *  \brief Must be called exactly once before any other functions in this file
 *
 *  Calls `pinMode()` as appropriate and initializes the display.
 */
void displaySetup() {
  pinMode(9, OUTPUT);
  digitalWrite(9, HIGH);
  display.begin();
  displayInit();
  display.setTextWrap(false);
}

/**
 *  \brief Must be called right after `Adafruit_PCD8544.begin()`.
 *
 *  Adafruit library sets the SPI clock to 4 MHz, but for some reason that's
 *  actually too fast for the PCD8544 (idk why) so none of the initialization
 *  commands in `PCD8544.begin()` actually get through, so we fix that here.
 */
void displayInit() {
  SPI.setClockDivider(SPI_CLOCK_DIV8); // set clock to 2 MHz
  display.command(0x21); // PCD8544_FUNCTIONSET | PCD8544_EXTENDEDINSTRUCTION
  display.command(0xC0); // PCD8544_SETVOP | 0x40
  display.command(0x14); // PCD8544_SETBIAS | 0x04
  display.command(0x20); // PCD8544_FUNCTIONSET
  display.command(0x0C); // PCD8544_DISPLAYCONTROL | PCD8544_DISPLAYNORMAL
  display.display();
}

/**
 *  \brief Displays the splash screen.
 */
void showSplash() {
  display.clearDisplay();
  display.setFont(&FreeSans9pt7b);
  printCentered("TENNIS", 29);
  display.setFont(NULL);
  display.display();
}

/**
 *  \brief Prompts for where player A starts.
 */
void showWhereStart() {
  display.clearDisplay();
  printCentered("Where does", 12);
  printCentered("player A", 20);
  printCentered("start?", 28);
  display.setCursor(0, 24);
  display.print('<');
  display.setCursor(79, 24);
  display.print('>');
  display.display();
}

/**
 *  \brief Prompts for which player serves first.
 *  \param state used only for which side the players are on
 */
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

/**
 *  \brief Displays the current score.
 *
 *  Displays the current set games, current game points, and who is serving.
 *
 *  \param state the current state
 */
void showState(State state) {
  // auto st = micros();
  // TODO: make this work for 5 sets
  static_assert(config::kSetsPerMatch == 3);

  display.clearDisplay();

  // set labels
  display.setCursor(2, 1);
  display.print("Set");
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

  // set scores
  display.drawChar(2, 13, 'A', BLACK, WHITE, 1);
  display.drawChar(2, 21, 'B', BLACK, WHITE, 1);

  display.setCursor(35, 13);
  for (auto a_games : state.pa_match_sets_games) {
    printInt2(a_games);
    display.print(' ');
  }

  display.setCursor(35, 21);
  for (auto b_games : state.pb_match_sets_games) {
    printInt2(b_games);
    display.print(' ');
  }

  display.drawFastHLine(0, 30, 84, BLACK);
  display.drawFastHLine(0, 32, 84, BLACK);

  // player sides
  display.drawChar(2, 37, p2c(state.west_player()), BLACK, WHITE, 1);
  display.drawChar(77, 37, p2c(state.east_player()), BLACK, WHITE, 1);

  // serve box
  display.drawRect((state.serve_side() == CourtSide::kWest ? 0 : 75), 35, 9, 11, BLACK);

  // game score
  display.drawFastHLine(39, 40, 6, BLACK);
  if (state.pa_game_points < 3 || state.pb_game_points < 3) {
    if (state.west_game_points() == 0) {
      display.setCursor(15, 37);
      display.print("Love");
    } else {
      display.setCursor(27, 37);
      printPointsNormal(state.west_game_points());
    }
    display.setCursor(46, 37);
    if (state.east_game_points() == 0) {
      display.print("Love");
    } else {
      printPointsNormal(state.east_game_points());
    }
  } else {
    switch (state.west_game_points() - state.east_game_points()) {
    case 0:
      display.drawFastHLine(39, 40, 6, WHITE);
      display.setCursor(27, 37);
      display.print("DEUCE");
      break;
    case -1:
      display.setCursor(27, 37);
      display.print(40);
      display.setCursor(46, 37);
      display.print("Adv");
      break;
    case 1:
      display.setCursor(21, 37);
      display.print("Adv");
      display.setCursor(46, 37);
      display.print(40);
      break;
    }
  }

  // auto end = micros();
  display.display();
  // debugDumpDisplayBuffer();
  // Serial.println(end - st);
}

void printInt2(uint8_t i) {
  if (i >= 10) {
    display.print(i);
  } else {
    display.print('0');
    display.print(i);
  }
}

void printPointsNormal(uint8_t p) {
  switch (p) {
  case 1:
    display.print(15);
    break;
  case 2:
    display.print(30);
    break;
  case 3:
    display.print(40);
    break;
  default:
    abort();
    break;
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
  int16_t x, y;
  uint16_t w, h;
  // ADAFRUIT Y U DO DIS
  display.getTextBounds((char*)s, 0, 0, &x, &y, &w, &h);
  display.setCursor((display.width() - (int16_t)w) / 2 - x, base);
  display.print(s);
}

/*
void debugDumpDisplayBuffer() {
  Serial.begin(230400);
  Serial.println("------------------------------------------------------------------------------------");
  for (auto y = 0; y < 48; ++y) {
    for (auto x = 0; x < 84; ++x) {
      if ((pcd8544_buffer[x+ (y/8)*LCDWIDTH] & _BV(y%8)) != 0) {
        Serial.print('#');
      } else {
        Serial.print(' ');
      }
    }
    Serial.println();
  }
}
*/
