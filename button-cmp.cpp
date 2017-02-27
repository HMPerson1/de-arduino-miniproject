constexpr int kPinWest = 2;
constexpr int kIntWest = digitalPinToInterrupt(kPinWest);
constexpr int kPinEast = 3;
constexpr int kIntEast = digitalPinToInterrupt(kPinEast);
constexpr uint8_t kDebounceClockCycles = 250;
constexpr uint8_t kDebounceTimeoutCount = 2;
static volatile CourtSide side;
static volatile bool timed_out;
void buttonSetup() {
  pinMode(kPinWest, INPUT_PULLUP);
  pinMode(kPinEast, INPUT_PULLUP);
  disableTimer();
  TCCR2A = _BV(WGM21);
  TCCR2B = _BV(CS22) | _BV(CS21) | _BV(CS20);
  OCR2A = kDebounceClockCycles;
}
CourtSide waitForButton() {
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  noInterrupts();
  attachInterrupt(kIntWest, onWestPress, FALLING);
  attachInterrupt(kIntEast, onEastPress, FALLING);
  sleep_enable();
  sleep_bod_disable();
  interrupts();
  sleep_cpu();
  timed_out = false;
  set_sleep_mode(SLEEP_MODE_PWR_SAVE);
  while (true) {
    noInterrupts();
    if (timed_out) break;
    sleep_enable();
    sleep_bod_disable();
    interrupts();
    sleep_cpu();
  }
  interrupts();
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
  if (digitalRead(kPinWest) == HIGH && digitalRead(kPinEast) == HIGH) startTimer();
  else disableTimer();
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
inline void disableTimer() {TIMSK2 = 0;}
