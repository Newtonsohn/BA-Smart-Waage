#include <Arduino.h>
#include "StateMachine.h"
#include "Logger.h"
#include "Properties.h"

StateMachine* stateMachine;

void setup() {
  // Check cause for ESP wakeup.
  Properties::wakeUpCauseIsTimer = esp_sleep_get_wakeup_cause() == ESP_SLEEP_WAKEUP_TIMER;

  Logger::enableLogging(Properties::loggingEnabled);
  Logger::log("Finished booting");

  stateMachine = new StateMachine();
}

void loop() {
  stateMachine->updateState();
}
