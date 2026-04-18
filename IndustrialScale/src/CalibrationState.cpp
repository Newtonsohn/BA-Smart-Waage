#include "CalibrationState.h"
#include "Properties.h"

#define CALIBRATION_WEIGHT_KG  1.0f

CalibrationState::CalibrationState() {}

void CalibrationState::enter() {
  Logger::log("Enter Calibration State");

  hw = HwContext::get();

  pinMode(Properties::BUTTON_CALIBRATION_PIN, INPUT_PULLUP);

  Logger::log("--- Step 1/2: Zero ---");
  Logger::log("Remove ALL weight from the scale (0 kg).");
  Logger::log("Then pull IO32 to GND to continue.");
}

void CalibrationState::update() {
  printAdcEverySecond();

  if (currentStep == Step::REMOVE_WEIGHT) {
    if (waitForButtonPress()) {
      Logger::log("Reading zero offset...");
      long rawEmpty = hw->averageScaleReading();
      Properties::zeroOffset = static_cast<float>(rawEmpty);
      Logger::log(("zeroOffset = " + std::to_string((long)Properties::zeroOffset) + " raw counts").c_str());

      currentStep = Step::PLACE_WEIGHT;
      Logger::log("--- Step 2/2: Known weight ---");
      Logger::log("Place 1.0 kg on the scale.");
      Logger::log("Then pull IO32 to GND to continue.");
    }
  } else if (currentStep == Step::PLACE_WEIGHT) {
    if (waitForButtonPress()) {
      Logger::log("Reading 1.0 kg offset...");
      long rawLoaded = hw->averageScaleReading();
      float rawDelta = static_cast<float>(rawLoaded) - Properties::zeroOffset;

      if (rawDelta == 0.0f) {
        Logger::log("ERROR: No change detected between 0 kg and 1 kg — calibration aborted.");
        Logger::log("Keeping previous calFactor.");
      } else {
        Properties::calFactor = rawDelta / CALIBRATION_WEIGHT_KG;
        Logger::log(("calFactor = " + std::to_string((long)Properties::calFactor) + " raw counts/kg").c_str());
        Properties::saveConfigToNVS();
        Logger::log("Calibration saved to NVS.");
      }

      currentStep = Step::DONE;
      Logger::log("--- Calibration complete ---");
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

void CalibrationState::printAdcEverySecond() {
  unsigned long now = millis();
  if (now - lastAdcPrintTime >= 1000) {
    lastAdcPrintTime = now;
    int32_t raw = hw->readADS1234();
    Logger::log(("ADC raw: " + std::to_string(raw)).c_str());
  }
}

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
