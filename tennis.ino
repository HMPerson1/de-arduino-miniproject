// -*- mode: c++ -*-
#include <Arduino.h>

#include "button.h"
#include "display.h"
#include "tennis.h"

/*
 * WIRING INFO
 * 13, 11 -- hardware SPI to display
 * 9      -- display led PWM
 * 7,6,5  -- display data
 * 2,3    -- buttons
 */

void setup() {
  buttonSetup();
  displaySetup();
  showSplash();
  delay(3500);
}

void loop() {
  tennisMain();
}
