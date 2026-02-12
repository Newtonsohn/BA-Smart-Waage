#include "MeasureState.h"
#include "Properties.h"
#include "StateType.h"

#define MILLISECONDS_PER_SECOND 1000
#define WEIGHT_CHANGE_THRESHOLD 0.9

MeasureState::MeasureState() {}

void MeasureState::enter() {
  Logger::log("Enter Measure State");

  hw = HwContext::get();

  // Calculate the weight in gramm
  Properties::currentWeight = ((hw->averageScaleReading() - Properties::zeroOffset) / Properties::calFactor) * MILLISECONDS_PER_SECOND;

  // Check if weight changed by more than 90% of the item weight.
  weightChanged = fabs(Properties::currentWeight - Properties::prevWeight) >= Properties::itemWeight * WEIGHT_CHANGE_THRESHOLD;

  Logger::log(("Current weight: " + std::to_string(Properties::currentWeight)).c_str());
  Logger::log(("Prev weight: " + std::to_string(Properties::prevWeight)).c_str());
  Logger::log(("Weight changed: " + std::string(weightChanged ? "true" : "false")).c_str());
}

void MeasureState::update() {}

void MeasureState::exit() {
  Logger::log("Exit Measure State");
}

StateType MeasureState::nextState() {
  // Check if heartbeat is due or weight changed
  if ((Properties::heartbeatTrigger - Properties::heartbeatCounter <= 0) || weightChanged) {
    Properties::heartbeatCounter = 0;
    return StateType::BLE_INIT;
  } else {
    return StateType::DEEP_SLEEP;
  }
}
