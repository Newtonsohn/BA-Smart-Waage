#pragma once

#include "State.h"
#include "StateType.h"
#include "Logger.h"

/**
 * @class StateMachine
 * @brief Represents the state machine.
 *
 * This class handles transition between the states, holds a pointer to the current state
 * and also calls the execution of (enter, update and exit) for the states.
 */
class StateMachine {
public:
  /**
   * @brief Constructor for SendDataState
   */
  StateMachine();

  /**
   * @brief This  copying due to CWE 398
   */
  StateMachine(const StateMachine&) = delete;
  StateMachine& operator=(const StateMachine&) = delete;

  /**
   * @brief Create the state machine
   */
  void createStateMachine();

  /**
   * @brief Checks if transition is required and calls the update methode within the current state
   */
  void updateState();

  /**
   * @brief Handles the transition to a new state
   * @param StateType The new state to transition to
   */
  void transitionTo(StateType newState);

private:
  // Identifiere for current state
  StateType currentStateType;

  // Pointer to the current state
  State* currentState;
};
