#include "CalibrationCheckState.h"
#include "Properties.h"
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Arduino.h>

#define CAL_X_MARGIN       10
#define CAL_Y_TITLE        20
#define CAL_Y_INSTRUCTION  50
#define CAL_Y_PROGRESS     65
#define CAL_PROGRESS_W     240
#define CAL_PROGRESS_H     20
#define CAL_TOTAL_STEPS    (CALIBRATION_WINDOW_MS / 1000)

#define CALIBRATION_WINDOW_MS  5000

CalibrationCheckState::CalibrationCheckState() {}

void CalibrationCheckState::enter() {
  Logger::log("Enter CalibrationCheck State");

  startTime = millis();
  hw = HwContext::get();

  while (Serial.available()) Serial.read();  // flush any pending bytes

  hw->display->fillScreen(GxEPD_WHITE);
  hw->display->setCursor(CAL_X_MARGIN, CAL_Y_TITLE);
  hw->display->setFont(&FreeSansBold12pt7b);
  hw->display->print("Calibration?");
  hw->display->setCursor(CAL_X_MARGIN, CAL_Y_INSTRUCTION);
  hw->display->setFont(&FreeSans9pt7b);
  hw->display->print("Press any key to calibrate");
  hw->display->drawRect(CAL_X_MARGIN, CAL_Y_PROGRESS, CAL_PROGRESS_W, CAL_PROGRESS_H, GxEPD_BLACK);
  hw->display->display(true);

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

    int barStep = CAL_TOTAL_STEPS - secondsLeft;
    int fillWidth = barStep * (CAL_PROGRESS_W / CAL_TOTAL_STEPS) - 2;
    if (fillWidth > 0) {
      hw->display->fillRect(CAL_X_MARGIN + 1, CAL_Y_PROGRESS + 1,
                            fillWidth, CAL_PROGRESS_H - 2, GxEPD_BLACK);
    }
    hw->display->display(true);
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

    if (Properties::calibrationValid) {
      auto hw = HwContext::get();
      float w = hw->measureWeight();
      char buf[40];
      snprintf(buf, sizeof(buf), "Current weight: %.1f g", w);
      Logger::log(buf);
    } else {
      Logger::log("Not calibrated — weight unavailable");
    }

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
