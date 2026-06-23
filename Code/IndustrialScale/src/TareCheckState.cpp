#include "TareCheckState.h"
#include "Properties.h"
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Arduino.h>

#define TARE_X_MARGIN       10
#define TARE_Y_TITLE        20
#define TARE_Y_INSTRUCTION  50
#define TARE_Y_PROGRESS     65
#define TARE_PROGRESS_W     240
#define TARE_PROGRESS_H     20
#define TARE_TOTAL_STEPS    (TARE_WINDOW_MS / 1000)
#define TARE_BUTTON_PIN     39       // GPIO39 tare button input

#define TARE_WINDOW_MS  5000

TareCheckState::TareCheckState() {}

static volatile bool tareButtonInterrupt = false;
static void IRAM_ATTR checkTareButtonInterrupt() {
  tareButtonInterrupt = true;
}

void TareCheckState::enter() {
  Logger::log("Enter TareCheck State");

  tareRequested = false;
  lastCountdownPrinted = -1;
  tareButtonInterrupt = false;
  hw = HwContext::get();

  while (Serial.available()) Serial.read();  // flush any pending bytes

  hw->display->fillScreen(GxEPD_WHITE);
  hw->display->setCursor(TARE_X_MARGIN, TARE_Y_TITLE);
  hw->display->setFont(&FreeSansBold12pt7b);
  hw->display->print("Taring?");
  hw->display->setCursor(TARE_X_MARGIN, TARE_Y_INSTRUCTION);
  hw->display->setFont(&FreeSans9pt7b);
  hw->display->print("Press any key to tare");
  hw->display->drawRect(TARE_X_MARGIN, TARE_Y_PROGRESS, TARE_PROGRESS_W, TARE_PROGRESS_H, GxEPD_BLACK);
  hw->display->display(true);

  Logger::log("--- Tare window open ---");
  Logger::log("Send any key via Serial within 5s to start taring.");
  Logger::log("5...");

  pinMode(TARE_BUTTON_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(TARE_BUTTON_PIN), checkTareButtonInterrupt, FALLING);

  // Start the countdown only after the (slow) initial e-ink refresh so the full
  // 5s window is available and the progress bar starts empty.
  startTime = millis();
}

void TareCheckState::update() {
  // Print countdown once per second
  unsigned long elapsed = millis() - startTime;
  int secondsLeft = (TARE_WINDOW_MS - (int)elapsed) / 1000;
  if (secondsLeft < 0) secondsLeft = 0;

  if (secondsLeft != lastCountdownPrinted) {
    lastCountdownPrinted = secondsLeft;
    if (secondsLeft > 0) {
      Logger::log((std::to_string(secondsLeft) + "...").c_str());
    }

    int barStep = TARE_TOTAL_STEPS - secondsLeft;
    int fillWidth = barStep * (TARE_PROGRESS_W / TARE_TOTAL_STEPS) - 2;
    if (fillWidth > 0) {
      hw->display->fillRect(TARE_X_MARGIN + 1, TARE_Y_PROGRESS + 1,
                            fillWidth, TARE_PROGRESS_H - 2, GxEPD_BLACK);
    }
    hw->display->display(true);
  }

  if (tareButtonInterrupt == true) {
    delay(50);  // debounce
    tareButtonInterrupt = false;
    tareRequested = true;
    detachInterrupt(digitalPinToInterrupt(TARE_BUTTON_PIN));
    return;
  }
  if (Serial.available()) {
    while (Serial.available()) Serial.read();  // consume
    tareRequested = true;
  }
}

void TareCheckState::exit() {
  detachInterrupt(digitalPinToInterrupt(TARE_BUTTON_PIN));

  if (tareRequested) {
    Logger::log("Tare requested — entering taring.");
  } else {
    Logger::log("No input — skipping taring.");
  }
  Logger::log("Exit TareCheck State");
}

StateType TareCheckState::nextState() {
  if (tareRequested) {
    return StateType::TARE;
  }
  if (millis() - startTime >= TARE_WINDOW_MS) {
    return StateType::BLE_INIT;
  }
  return StateType::TARE_CHECK;
}
