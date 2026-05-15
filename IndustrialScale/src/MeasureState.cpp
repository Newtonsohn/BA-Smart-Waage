#include "MeasureState.h"
#include "Properties.h"
#include "StateType.h"

#define WEIGHT_CHANGE_THRESHOLD 0.9

MeasureState::MeasureState() {}

void MeasureState::enter() {
  Logger::log("Enter Measure State");

  hw = HwContext::get();

  if (!Properties::calibrationValid) {
    Logger::log("Warning: not calibrated — weight reported as 0");
    Properties::currentWeight = 0.0f;
  } else {
    Properties::currentWeight = hw->measureWeight();
  }

  weightChanged = fabs(Properties::currentWeight - Properties::prevWeight) >= Properties::itemWeight * WEIGHT_CHANGE_THRESHOLD;

  char buf[60];
  snprintf(buf, sizeof(buf), "Weight: %.1f g  (prev: %.1f g  changed: %s)",
           Properties::currentWeight, Properties::prevWeight, weightChanged ? "yes" : "no");
  Logger::log(buf);
}

void MeasureState::update() {}

void MeasureState::exit() {
  Logger::log("Exit Measure State");
}

StateType MeasureState::nextState() {
  if ((Properties::heartbeatTrigger - Properties::heartbeatCounter <= 0) || weightChanged) {
    Properties::heartbeatCounter = 0;
    return StateType::BLE_INIT;
  } else {
    return StateType::DEEP_SLEEP;
  }
}
