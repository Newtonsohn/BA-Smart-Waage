#pragma once

#include <memory>
#include "HwContext.h"
#include "State.h"
#include "Logger.h"

/**
 * @class ConfigurationState
 * @brief Loads the Configuration for the scale from the Gateway
 *
 * If Configuration is loaded sucessfully, transition to the next state occures.
 * The next state depends whether there configuration was successful and there was not timeout 
 */
class ConfigurationState : public State {
public:
  /**
  
   * @brief Constructor for ConfigurationState
   */
  ConfigurationState();

  /**
   * @brief Called when entering this state. Initializes the required hardware.
   */
  void enter() override;

  /**
   * @brief Waits until the configuration is received and parsed successfully, than updates the E-Ink Display with the new configuration.
   */
  void update() override;

  /**
   * @brief Called on exit
   */
  void exit() override;

  /**
   * @brief Determines the next state depnding on the success of the configuration. Handles timeout.
   * @return StateType of the next State. 
   */
  StateType nextState() override;

private:
  // Shared hardware context. Used to access required Hardware/ Peripherals.
  std::shared_ptr<HwContext> hw;

  // Start time to handle timeout
  unsigned long startTime = 0;

  // Indicates if configuration was loaded successfully.
  bool configurationSuccess = false;

  /**
   * @brief Displays the current loaded configuration on the attached E-ink Display.
   */
  void updateDisplayWithConfiguration();

  /**
   * @brief Handles Failures and set error message.
   * @param errorMessage Error message that needs to be handled
   */
  void handleFailure(const String& errorMessage);

  /**
   * @brief Parses the config out of the JSON received by gateway.
   */
  bool parseConfiguration();
};
