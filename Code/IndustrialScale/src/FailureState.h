#pragma once

#include <memory>
#include "HwContext.h"
#include "State.h"
#include "Logger.h"

/**
 * @class FailureState
 * @brief State handles failure events
 *
 * This State prints the writes the failure message to the E-Ink display and sends the device into deep sleep forever.
 * The device can then only be reset by a power cycle.
 */
class FailureState : public State {
public:
  /**
   * @brief Constructor for FailureState
   */
  FailureState();

  /**
   * @brief Display failure on E-Ink display and put devide to sleep
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

  // Prints the current failure message, the barcode and the MAC-Address on the E-Ink display.
  void updateDisplayWithFailure();
};
