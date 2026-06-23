#pragma once

#include <memory>
#include "HwContext.h"
#include "State.h"
#include "Logger.h"

/**
 * @class TareCheckState
 * @brief Opens a short window in which the user can request a taring.
 *
 * Mirrors CalibrationCheckState: a 5 second window is shown on the display with a
 * progress bar. Pressing the tare button (GPIO39) or sending any key via Serial
 * within the window requests a taring. On timeout the taring is skipped.
 */
class TareCheckState : public State {
public:
  TareCheckState();

  void enter() override;
  void update() override;
  void exit() override;
  StateType nextState() override;

private:
  std::shared_ptr<HwContext> hw;
  unsigned long startTime = 0;
  bool tareRequested = false;
  int lastCountdownPrinted = -1;
};
