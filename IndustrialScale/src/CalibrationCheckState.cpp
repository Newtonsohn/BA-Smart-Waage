#include "CalibrationCheckState.h"
#include "Properties.h"
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>

#define CALIBRATION_WINDOW_MS   5000
#define DISPLAY_X_MARGIN        10
#define DISPLAY_Y_TITLE         25
#define DISPLAY_Y_INSTRUCTION   60
#define DISPLAY_Y_COUNTDOWN     100

CalibrationCheckState::CalibrationCheckState() {}

void CalibrationCheckState::enter() {
  Logger::log("Enter CalibrationCheck State");

  hw = HwContext::get();

  pinMode(Properties::BUTTON_CALIBRATION_PIN, INPUT_PULLUP);

  startTime = millis();

  hw->display->fillScreen(GxEPD_WHITE);
  hw->display->setFont(&FreeSansBold12pt7b);
  hw->display->setCursor(DISPLAY_X_MARGIN, DISPLAY_Y_TITLE);
  hw->display->print("Calibration");

  hw->display->setFont(&FreeSans9pt7b);
  hw->display->setCursor(DISPLAY_X_MARGIN, DISPLAY_Y_INSTRUCTION);
  hw->display->print("Press IO32 within 5s");
  hw->display->setCursor(DISPLAY_X_MARGIN, DISPLAY_Y_COUNTDOWN);
  hw->display->print("to start calibration");
  hw->display->display(true);
}

void CalibrationCheckState::update() {
  if (digitalRead(Properties::BUTTON_CALIBRATION_PIN) == LOW) {
    delay(50);  // debounce
    if (digitalRead(Properties::BUTTON_CALIBRATION_PIN) == LOW) {
      calibrationRequested = true;
      // Wait for release before leaving
      while (digitalRead(Properties::BUTTON_CALIBRATION_PIN) == LOW) {
        delay(10);
      }
    }
  }
}

void CalibrationCheckState::exit() {
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
