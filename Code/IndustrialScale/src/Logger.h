#pragma once

#include <Arduino.h>

/**
 * @class Logger
 * @brief Logging utilty
 *
 * This class provides a simple logging utility that allows logging to the serial monitor.
 */
class Logger {
public:
  /**
   * @brief Enable or disable logging
   * @param value Enable or disable logging
   */
  static void enableLogging(bool value);

  /**
   * @brief log output to serial monitor
   * @param message Message to log
   */
  static void log(const char* message);
};
