#pragma once

#include <memory>
#include "HwContext.h"
#include "State.h"
#include "Logger.h"

/**
 * @class TareState
 * @brief State executes the taring
 *
 * In this state the raw zero offset of each channel is re-captured with the scale
 * empty and stored to NVS, so the empty scale reads 0 g. The calibrated spanFactor
 * (corner balancing) is left untouched.
 */
class TareState : public State {
public:
  /**
   * @brief Constructor for TareState
   */
  TareState();

  /**
   * @brief Initialize the required hardware. Execute the taring.
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
   * @brief Transition to the BleInitState
   * @return StateType of the next State. 
   */
  StateType nextState() override;

private:
  // Shared hardware context. Used to acceess required Hardware/ Peripherals.
  std::shared_ptr<HwContext> hw;

  /**
   * @brief Write title text on E-Ink display
   */
  void updateDisplayInitText();

  /**
   * @brief Write instruction text on E-Ink display
   */
  void updateDisplayAddWeight();

  /**
   * @brief Draw the (empty) progress bar outline on the E-Ink display
   */
  void drawProgressBarOutline();

  /**
   * @brief Re-capture the raw zero offset of each channel (the actual taring)
   *        and persist it to NVS, advancing the progress bar per channel.
   */
  void performTare();

  /**
   * @brief Write final message to display
   */
  void updateDisplayTaringComplete();
};
