#include "CalibrationState.h"
#include "Properties.h"
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>

#define CALIBRATION_WEIGHT_KG   1.0f
#define DISPLAY_X_MARGIN        10
#define DISPLAY_Y_TITLE         25
#define DISPLAY_Y_LINE1         55
#define DISPLAY_Y_LINE2         80
#define DISPLAY_Y_LINE3         105

CalibrationState::CalibrationState() {}

void CalibrationState::enter() {
  Logger::log("Enter Calibration State");

  hw = HwContext::get();

  // Button pin already set up in CalibrationCheckState, but set again to be safe
  pinMode(Properties::BUTTON_CALIBRATION_PIN, INPUT_PULLUP);

  showStep1();
}

void CalibrationState::update() {
  if (currentStep == Step::REMOVE_WEIGHT) {
    if (waitForButtonPress()) {
      long rawEmpty = hw->averageScaleReading();
      Properties::zeroOffset = static_cast<float>(rawEmpty);

      Logger::log(("Calibration: zeroOffset = " + std::to_string(Properties::zeroOffset)).c_str());

      currentStep = Step::PLACE_WEIGHT;
      showStep2();
    }
  } else if (currentStep == Step::PLACE_WEIGHT) {
    if (waitForButtonPress()) {
      long rawLoaded = hw->averageScaleReading();
      float rawDelta = static_cast<float>(rawLoaded) - Properties::zeroOffset;

      if (rawDelta == 0.0f) {
        Logger::log("Calibration error: no change detected, aborting");
        // Keep existing calFactor rather than saving 0
      } else {
        Properties::calFactor = rawDelta / CALIBRATION_WEIGHT_KG;
        Logger::log(("Calibration: calFactor = " + std::to_string(Properties::calFactor)).c_str());
        Properties::saveConfigToNVS();
        Logger::log("Calibration saved to NVS");
      }

      currentStep = Step::DONE;
      showDone();
      delay(2000);
    }
  }
}

void CalibrationState::exit() {
  Logger::log("Exit Calibration State");
}

StateType CalibrationState::nextState() {
  if (currentStep == Step::DONE) {
    return StateType::BLE_INIT;
  }
  return StateType::CALIBRATION;
}

// Blocks until button is pressed and released. Returns true when done.
bool CalibrationState::waitForButtonPress() {
  if (digitalRead(Properties::BUTTON_CALIBRATION_PIN) == LOW) {
    delay(50);  // debounce
    if (digitalRead(Properties::BUTTON_CALIBRATION_PIN) == LOW) {
      while (digitalRead(Properties::BUTTON_CALIBRATION_PIN) == LOW) {
        delay(10);
      }
      return true;
    }
  }
  return false;
}

void CalibrationState::showStep1() {
  hw->display->fillScreen(GxEPD_WHITE);
  hw->display->setFont(&FreeSansBold12pt7b);
  hw->display->setCursor(DISPLAY_X_MARGIN, DISPLAY_Y_TITLE);
  hw->display->print("Calibration 1/2");

  hw->display->setFont(&FreeSans9pt7b);
  hw->display->setCursor(DISPLAY_X_MARGIN, DISPLAY_Y_LINE1);
  hw->display->print("Remove ALL weight");
  hw->display->setCursor(DISPLAY_X_MARGIN, DISPLAY_Y_LINE2);
  hw->display->print("from the scale,");
  hw->display->setCursor(DISPLAY_X_MARGIN, DISPLAY_Y_LINE3);
  hw->display->print("then press IO32.");
  hw->display->display(true);
}

void CalibrationState::showStep2() {
  hw->display->fillScreen(GxEPD_WHITE);
  hw->display->setFont(&FreeSansBold12pt7b);
  hw->display->setCursor(DISPLAY_X_MARGIN, DISPLAY_Y_TITLE);
  hw->display->print("Calibration 2/2");

  hw->display->setFont(&FreeSans9pt7b);
  hw->display->setCursor(DISPLAY_X_MARGIN, DISPLAY_Y_LINE1);
  hw->display->print("Place 1.0 kg on");
  hw->display->setCursor(DISPLAY_X_MARGIN, DISPLAY_Y_LINE2);
  hw->display->print("the scale,");
  hw->display->setCursor(DISPLAY_X_MARGIN, DISPLAY_Y_LINE3);
  hw->display->print("then press IO32.");
  hw->display->display(true);
}

void CalibrationState::showDone() {
  hw->display->fillScreen(GxEPD_WHITE);
  hw->display->setFont(&FreeSansBold12pt7b);
  hw->display->setCursor(DISPLAY_X_MARGIN, DISPLAY_Y_TITLE);
  hw->display->print("Calibration done!");

  hw->display->setFont(&FreeSans9pt7b);
  hw->display->setCursor(DISPLAY_X_MARGIN, DISPLAY_Y_LINE1);
  hw->display->print("Values saved.");
  hw->display->display(true);
}
