#pragma once

#include <memory>
#include "HwContext.h"
#include "State.h"
#include "Logger.h"

/**
 * @class CalibrationCheckState
 * @brief Gives a 5-second window on cold boot to enter calibration mode.
 *
 * If IO32 is pressed within 5 seconds, transitions to CalibrationState.
 * Otherwise transitions directly to BleInitState.
 */
class CalibrationCheckState : public State {
public:
  CalibrationCheckState();

  void enter() override;
  void update() override;
  void exit() override;
  StateType nextState() override;

private:
  std::shared_ptr<HwContext> hw;

  unsigned long startTime = 0;
  bool calibrationRequested = false;
};
