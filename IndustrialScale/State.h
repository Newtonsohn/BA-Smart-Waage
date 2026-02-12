#pragma once

#include "StateType.h"

/**
 * @class State
 * @brief Abstract base class for the states of the state machine.
 *
 * In this state the ADC is set up. When it is ready, transition to the next state occurs.
 * The next state depends whether there was a power cycle or a wake up from deep sleep.
 */
class State {
public:
  /**
   * @brief Virtual deconstructor (allows proper cleanup)
   */
  virtual ~State() {}

  /**
   * @brief Called when entering a state.
   */
  virtual void enter() = 0;

  /**
   * @brief Repeatedly called while state active
   */
  virtual void update() = 0;

  /**
   * @brief Called when exiting a state.
   */
  virtual void exit() = 0;

  /**
   * @brief Determines the next state.
   * @return StateType of the next State.
   */
  virtual StateType nextState() = 0;
};
