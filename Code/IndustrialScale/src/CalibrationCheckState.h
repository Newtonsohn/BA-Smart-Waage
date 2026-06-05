#pragma once

#include <memory>
#include "HwContext.h"
#include "State.h"
#include "Logger.h"

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
  int lastCountdownPrinted = -1;
};
