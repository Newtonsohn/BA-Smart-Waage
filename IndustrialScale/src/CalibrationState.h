#pragma once

#include <memory>
#include "HwContext.h"
#include "State.h"
#include "Logger.h"

/**
 * @class CalibrationState
 * @brief Two-step calibration: zero (empty scale) then known weight.
 *
 * Step 1 — Remove all weight, press IO32  → captures zeroOffset
 * Step 2 — Place 1.0 kg,      press IO32  → captures calFactor
 * Results are saved to NVS immediately.
 */
class CalibrationState : public State {
public:
  CalibrationState();

  void enter() override;
  void update() override;
  void exit() override;
  StateType nextState() override;

private:
  enum class Step { REMOVE_WEIGHT, PLACE_WEIGHT, DONE };

  std::shared_ptr<HwContext> hw;
  Step currentStep = Step::REMOVE_WEIGHT;
  unsigned long lastAdcPrintTime = 0;

  bool waitForButtonPress();
  void printAdcEverySecond();
};
