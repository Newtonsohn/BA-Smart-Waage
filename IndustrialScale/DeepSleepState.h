#pragma once

#include "HwContext.h"
#include "State.h"
#include "Logger.h"


/**
 * @class DeepSleepState
 * @brief State puts device into deep sleep.
 *
 * In this state the Device is sent into deep sleep. The amount of time the device is put 
 * to sleep depends on the configurated value and the elapsed time since the device booted. 
 */
class DeepSleepState : public State {
public:
  /**
   * @brief Constructor for DeepSleepState
   */
  DeepSleepState();

  /**
   * @brief Calculates the timer and puts device into deep sleep.
   */
  void enter() override;

  /**
   * @brief Stump methode never called due to deep sleep initiating a fresh boot after the timer ends.
   */
  void update() override;

  /**
   * @brief Stump methode never called due to deep sleep initiating a fresh boot after the timer ends.
   */
  void exit() override;

  /**
   * @brief Stump methode never called due to deep sleep initiating a fresh boot after the timer ends.
   */
  StateType nextState() override;

private:
  // Shared hardware context. Used to acceess required Hardware/ Peripherals.
  std::shared_ptr<HwContext> hw;
};
