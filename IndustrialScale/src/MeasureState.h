#pragma once

#include <memory>
#include "HwContext.h"
#include "State.h"
#include "Logger.h"

/**
 * @class MeasureState
 * @brief This state executes the measurement.
 *
 * In this state the measurement is executed. It checks if the weight changed by more than 90% of the item weight
 * since the last measuerement or a heartbeat is due according to the configured heartbeat interval.
 */
class MeasureState : public State {
public:
  /**
   * @brief Constructor for MeasureState
   */
  MeasureState();

  /**
   * @brief Exececution of measurement, check weight changed by more than 90% of the item weight
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
   * @brief Determines the next state. Next state depends on heartbeat trigger and weight changed.
   * @return StateType of the next State
   */
  StateType nextState() override;

private:
  // Shared hardware context. Used to acceess required Hardware/ Peripherals.
  std::shared_ptr<HwContext> hw;

  // Flag that indicates if weight changed
  bool weightChanged = false;
};
