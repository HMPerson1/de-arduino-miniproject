/**
 *   \file button.cpp
 *   \brief Interfaces with the buttons.
 *
 *  ## Notes
 *
 *   - East side (INT0) _technically_ has higher priority than west side (INT1)
 *     but this really only matters if both are pressed within the same clock
 *     cycle (i.e. within 62.5 ns of eachother)
 *
 *   - TIMER2 controls PWM on pin 3 and 11, which we don't use for anything, so
 *     it's safe(-ish) to commandeer it here for our own purposes. TIMER2 is
 *     also the only timer that can wake the CPU during sleep.
 *
 *   - Checking for low-level interrupts happens only at the end of an
 *     instruction and edge-triggered interrupts are only guaranteed to fire
 *     if the pulse lasts longer than one clock period, so super short presses
 *     may not be detected.
 *
 *  Reference: <http://gammon.com.au/interrupts>
 */

#include "button.h"

#include <inttypes.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

#include <Arduino.h>

#include "common.h"

// pins'n'stuff
constexpr int kPinWest = 2;
constexpr int kIntWest = digitalPinToInterrupt(kPinWest);
constexpr int kPinEast = 3;
constexpr int kIntEast = digitalPinToInterrupt(kPinEast);
static_assert(kIntWest != NOT_AN_INTERRUPT && kIntEast != NOT_AN_INTERRUPT);

constexpr uint8_t kDebounceClockCycles = 250; // 16 ms
constexpr uint8_t kDebounceTimeoutCount = 2; // 2 * 16 ms = 32 ms

// used for communication between ISRs and the main program
static volatile CourtSide side;
static volatile bool timed_out;

static uint8_t timeout_count;

static void onWestPress();
static void onEastPress();
static void onEdge();
static void startTimer();
static void disableTimer();

/**
 *  \brief Must be called exactly once before any other functions in this file.
 *
 *  Calls `pinMode()` for the button pins and sets up TIMER2.
 */
void buttonSetup() {
  pinMode(kPinWest, INPUT_PULLUP);
  pinMode(kPinEast, INPUT_PULLUP);
  disableTimer();
  // mode CTC; OC2x disconnected; prescaler 1024
  TCCR2A = _BV(WGM21);
  TCCR2B = _BV(CS22) | _BV(CS21) | _BV(CS20);
  OCR2A = kDebounceClockCycles;
}

/**
 *  \brief Sleeps until a button is pressed.
 *
 *  Attaches interrupts to both button pins and sleeps the CPU. Upon interrupt,
 *  waits for both buttons become unpressed, then returns which button was
 *  pressed.
 *
 *  It is assumed that both buttons are currently 'up' (i.e. `HIGH`) when this
 *  function is called.
 *
 *  \return which button was pressed
 */
CourtSide waitForButton() {
  // mfw this function is over 50% comments

  // ~~~ wait for button pressed ~~~
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  noInterrupts();
  // critical section
  // if we get interrupted after one of the `attachInterrupt()`s but before the
  // sleep begins, we would miss the event as it's what's supposed to wake us;
  // thus interrupts must be disabled here
  // we _could_ have the interrupts set a flag after they fire and just check
  // that flag right before sleeping, but this is simpler
  attachInterrupt(kIntWest, onWestPress, FALLING);
  attachInterrupt(kIntEast, onEastPress, FALLING);
  sleep_enable();
  sleep_bod_disable();
  interrupts();
  sleep_cpu();
  // INT0/1, pin change int, TWI addr match, and WDT are the only things that
  // can wake us from a power-down sleep. We're not using pin change ints, TWI,
  // or WDT, so after we've been awoken, it must have been either INT0/1,
  // meaning a button was pressed
  // `sleep_disable()` is called from the ISRs, so we don't have to worry about
  // that here

  // ~~~ wait for both buttons released ~~~
  timed_out = false;
  // whenever a button level changes, we start the timer only if both buttons
  // are now up, otherwise stop the timer (see `onEdge()`)
  set_sleep_mode(SLEEP_MODE_PWR_SAVE);
  while (true) {
    noInterrupts();
    // critical section
    if (timed_out) break;
    sleep_enable();
    sleep_bod_disable();
    interrupts();
    sleep_cpu();
    // just keep sleeping and let the ISRs do their thing until the timer
    // finally fires
  }
  interrupts();
  // timer fired; so both buttons have stayed high for 10 ms
  // again, `sleep_disable()` and `detachInterrupt()` are called from the ISR,
  // so we can just return
  return side;
}

void onWestPress() {
  onEdge();
  attachInterrupt(kIntWest, onEdge, CHANGE);
  attachInterrupt(kIntEast, onEdge, CHANGE);
  side = CourtSide::kWest;
}

void onEastPress() {
  onEdge();
  attachInterrupt(kIntWest, onEdge, CHANGE);
  attachInterrupt(kIntEast, onEdge, CHANGE);
  side = CourtSide::kEast;
}

void onEdge() {
  sleep_disable();
  if (digitalRead(kPinWest) == HIGH && digitalRead(kPinEast) == HIGH) {
    startTimer();
  } else {
    disableTimer();
  }
}

ISR(TIMER2_COMPA_vect) {
  sleep_disable();
  timeout_count++;
  if (timeout_count >= kDebounceTimeoutCount){
    timed_out = true;
    disableTimer();
    detachInterrupt(kIntWest);
    detachInterrupt(kIntEast);
  }
}

inline void startTimer() {
  TCNT2 = 0;
  TIFR2 = 0;
  TIMSK2 = _BV(OCIE2A);
  timeout_count = 0;
}

inline void disableTimer() {
  TIMSK2 = 0;
}
