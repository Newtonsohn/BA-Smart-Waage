#pragma once

#include <memory>
#include "HwContext.h"
#include "State.h"
#include "Logger.h"

/**
 * @class AdcInitState
 * @brief State initializes the analog digital converter (ADC).
 *
 * In this state the ADC is set up. When it is ready, transition to the next state occures.
 * The next state depends whether there was a power cycle or a wake up from deep sleep.
 */
class AdcInitState : public State {
public:
  /**
   * @brief Constructor for AdcInitState
   */
  AdcInitState();

  /**
   * @brief Called when entering this state. Initializes the ADC.
   */
  void enter() override;

  /**
   * @brief Waiting until the ADC is ready.
   */
  void update() override;

  /**
   * @brief Called on exit
   */
  void exit() override;

  /**
   * @brief Determines the next state depending on ADC initialization and type of wake-up.
   * @return StateType of the next State
   */
  StateType nextState() override;

private:
  // Shared hardware context. Used to acceess required Hardware/ Peripherals.
  std::shared_ptr<HwContext> hw;

  // Flag indicates if ADC initialization is done.
  bool adcIsReady = false;
};
