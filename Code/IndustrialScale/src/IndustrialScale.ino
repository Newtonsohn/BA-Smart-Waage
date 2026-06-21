#include <Arduino.h>
#include "esp_system.h"
#include "StateMachine.h"
#include "Logger.h"
#include "Properties.h"

StateMachine* stateMachine;

void setup() {
  // Check cause for ESP wakeup.
  Properties::wakeUpCauseIsTimer = esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER;

    Logger::enableLogging(Properties::loggingEnabled);
  Logger::log("Finished booting");

  char resetBuf[48];
  snprintf(resetBuf, sizeof(resetBuf), "Reset reason: %d", static_cast<int>(esp_reset_reason()));
  Logger::log(resetBuf);

  stateMachine = new StateMachine();
}

void loop() {
  stateMachine->updateState();
}
