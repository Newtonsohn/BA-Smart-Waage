#pragma once

#include <memory>
#include "HwContext.h"
#include "State.h"
#include "Logger.h"

/**
 * @class SendDataState
 * @brief State sends data to gateway over BLE.
 *
 * In this state the device sends the measurement data to its subscriber over BLE.
 */
class SendDataState : public State {
public:
  /**
   * @brief Constructor for SendDataState
   */
  SendDataState();

  /**
   * @brief Called when entering this state. Initializes the required hardware.
   */
  void enter() override;

  /**
   * @brief Waits until the gateway subscribed, than sends the measurement data to the gateway.
   */
  void update() override;

  /**
   * @brief Called on exit
   */
  void exit() override;

  /**
   * @brief Determines the next state depnding on the succesfull transmission of the data. Handles timeout.
   * @return StateType of the next State. 
   */
  StateType nextState() override;

private:
  // Shared hardware context. Used to acceess required Hardware/ Peripherals.
  std::shared_ptr<HwContext> hw;

  // Start time to handle timeout
  unsigned long startTime = 0;

  // Indicates if data are sent successfully.
  bool sendDataSuccess = false;

  /**
   * @brief Handles Failures and set error message.
   * @param errorMessage Error message that needs to be handled
   */
  void handleFailure(const String& errorMessage);
};
