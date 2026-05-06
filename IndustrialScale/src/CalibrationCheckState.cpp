#include "CalibrationCheckState.h"
#include "Properties.h"

#define CALIBRATION_WINDOW_MS  5000

CalibrationCheckState::CalibrationCheckState() {}

void CalibrationCheckState::enter() {
  Logger::log("Enter CalibrationCheck State");

  startTime = millis();

  while (Serial.available()) Serial.read();  // flush any pending bytes

  Logger::log("--- Calibration window open ---");
  Logger::log("Send any key via Serial within 5s to start calibration.");
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

  if (Serial.available()) {
    while (Serial.available()) Serial.read();  // consume
    calibrationRequested = true;
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
