#pragma once

#include <memory>
#include "HwContext.h"
#include "State.h"
#include "Logger.h"

class CalibrationState : public State {
public:
  CalibrationState();



  void enter() override;  // blocking — runs full calibration sequence
  void update() override;
  void exit() override;
  StateType nextState() override;

private:
  std::shared_ptr<HwContext> hw;

  // Block until any byte is received on Serial. Prints live ADC while waiting.
  bool waitForButton(const char* msg);


    
  // Print current raw ADC value for all 4 channels.
  void printAllChannels();

  // Draw the current calibration step. The arrow is shown while ADC sampling is active.
  void updateCalibrationDisplay(const char* stepText, bool showArrow);

  void runCalibrationZero();
};
