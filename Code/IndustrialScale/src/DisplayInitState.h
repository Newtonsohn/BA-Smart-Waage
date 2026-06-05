#pragma once

#include <memory>
#include "HwContext.h"
#include "State.h"
#include "Logger.h"

/**
 * @class DisplayInitState
 * @brief State initializes the E-Ink display.
 *
 * In this state initializes the display and sets it up for further use by other states.
 */
class DisplayInitState : public State {
public:
  /**
   * @brief Constructor for DisplayInitState
   */
  DisplayInitState();

  /**
   * @brief Initialization and setup of the display .
   */
  void enter() override;

  /**
   * @brief Stump methode
   */
  void update() override;

  /**
   * @brief Called on exit
   */
  void exit() override;

  /**
   * @brief Determines the next state. Routes to failure state if sendDataFailed is set to true.
   * @return StateType of the next State
   */
  StateType nextState() override;

private:
  // Shared hardware context. Used to acceess required Hardware/ Peripherals.
  std::shared_ptr<HwContext> hw;
};
