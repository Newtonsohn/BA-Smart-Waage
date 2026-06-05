#pragma once

#include <memory>
#include "HwContext.h"
#include "State.h"
#include "Logger.h"

/**
 * @class TareState
 * @brief State exectues the taring
 *
 * In this state the taring is done. Also the calibration- and the zero factor are calculated and stored 
 * for further use
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
   * @brief Print progress bar on E-Ink display
   */
  void updateDisplayProgressBar();

  /**
   * @brief Write final message to display
   */
  void updateDisplayTaringComplete();
};
