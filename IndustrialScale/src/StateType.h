#pragma once

/**
 * @enum StateType
 * @brief Enumeration of all the possible states of state machine
 */
enum class StateType {
  DISPLAY_INIT,
  ADC_INIT,
  TARE,
  MEASURE,
  BLE_INIT,
  CONFIGURATION,
  SEND_DATA,
  DEEP_SLEEP,
  FAILURE
};
