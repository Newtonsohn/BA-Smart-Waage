#pragma once

#include <memory>
#include "HwContext.h"
#include "State.h"
#include "Logger.h"

/**
 * @class BleInitState
 * @brief State initializes the bluetooth low energy (BLE) Hardware.
 *
 * In this state the BLE and its services are set up. When it is ready, transition to the next state occures.
 */
class BleInitState : public State {
public:
  /**
   * @brief Constructor for BleInitState
   */
  BleInitState();

  /**
   * @brief Called when entering this state. Initializes the BLE module.
   */
  void enter() override;

  /**
   * @brief Waiting until the BLE services are ready.
   */
  void update() override;

  /**
   * @brief Called on exit
   */
  void exit() override;

  /**
   * @brief Determines the next state depending on type of wake-up.
   * @return StateType of the next State 
   */
  StateType nextState() override;

private:
  // Shared hardware context. Used to acceess required Hardware/ Peripherals.
  std::shared_ptr<HwContext> hw;
};
