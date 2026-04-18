#include "CalibrationCheckState.h"
#include "Properties.h"

#define CALIBRATION_WINDOW_MS  5000

CalibrationCheckState::CalibrationCheckState() {}

void CalibrationCheckState::enter() {
  Logger::log("Enter CalibrationCheck State");

  hw = HwContext::get();

  pinMode(Properties::BUTTON_CALIBRATION_PIN, INPUT_PULLUP);

  startTime = millis();

  // Button is INPUT_PULLUP → pull IO32 to GND to trigger
  Logger::log("--- Calibration window open ---");
  Logger::log("Pull IO32 to GND within 5s to start calibration.");
  Logger::log("5...");
}

void CalibrationCheckState::update() {
  // Print countdown once per second
  unsigned long elapsed = millis() - startTime;
  int secondsLeft = (CALIBRATION_WINDOW_MS - (int)elapsed) / 1000;
  if (secondsLeft < 0) secondsLeft = 0;

  if (secondsLeft != lastCountdownPrinted) {
    lastCountdownPrinted = secondsLeft;
    if (secondsLeft > 0) {
      Logger::log((std::to_string(secondsLeft) + "...").c_str());
    }
  }

  // Check for button press (active LOW with INPUT_PULLUP)
  if (digitalRead(Properties::BUTTON_CALIBRATION_PIN) == LOW) {
    delay(50);  // debounce
    if (digitalRead(Properties::BUTTON_CALIBRATION_PIN) == LOW) {
      calibrationRequested = true;
      while (digitalRead(Properties::BUTTON_CALIBRATION_PIN) == LOW) {
        delay(10);
      }
    }
  }
}

void CalibrationCheckState::exit() {
  if (calibrationRequested) {
    Logger::log("Calibration requested — entering calibration.");
  } else {
    Logger::log("No input — skipping calibration.");
  }
  Logger::log("Exit CalibrationCheck State");
}

StateType CalibrationCheckState::nextState() {
  if (calibrationRequested) {
    return StateType::CALIBRATION;
  }
  if (millis() - startTime >= CALIBRATION_WINDOW_MS) {
    return StateType::BLE_INIT;
  }
  return StateType::CALIBRATION_CHECK;
}
