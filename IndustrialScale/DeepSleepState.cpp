#include "DeepSleepState.h"
#include "esp_sleep.h"
#include "Properties.h"
#include <string>

#define BLE_CONNECTION_ID 0

#define MILLISECONDS_PER_SECOND 1000
#define MICROSECONDS_PER_SECOND 1000000

DeepSleepState::DeepSleepState() {}

void DeepSleepState::enter() {
  Logger::log("Enter DeepSleep State");

  hw = HwContext::get();

  // Disconnect any BLE connection if connected
  if (Properties::deviceConnected) {
    hw->bleServer->disconnect(BLE_CONNECTION_ID);
    Logger::log("BLE Disconnected");
  }

  // Store values to NVS before going to deep sleep
  Properties::saveConfigToNVS();

  uint32_t total = Properties::updateInterval;
  uint32_t elapsed = millis() / MILLISECONDS_PER_SECOND;

  // Sleep duration is calculated by the configured update interval which is reduce by the time since the device booted. can not be negative
  uint32_t sleep_duration = (elapsed >= total) ? 0 : (total - elapsed);

  // Logging stays in seconds
  Logger::log(("Going to sleep for " + std::to_string(sleep_duration) + " seconds").c_str());
  Logger::log(("Current heartbeat counter " + std::to_string(Properties::heartbeatCounter) + " Trigger: " + std::to_string(Properties::heartbeatTrigger)).c_str());

  // Convert only at the very end
  esp_sleep_enable_timer_wakeup(static_cast<uint64_t>(sleep_duration) * MICROSECONDS_PER_SECOND);
  esp_deep_sleep_start();
}
void DeepSleepState::update() {
  // Stump methode never called due to deep sleep initiating a fresh boot after the timer ends
  Logger::log("Warning: update() should never be called in DeepSleepState");
}

void DeepSleepState::exit() {
  // Stump methode never called due to deep sleep initiating a fresh boot after the timer ends
  Logger::log("Warning: exit() should never be called in DeepSleepState");
}

StateType DeepSleepState::nextState() {
  // Stump methode never called due to deep sleep initiating a fresh boot after the timer ends
  Logger::log("Warning: nexState() should never be called in DeepSleepState");
  return StateType::DEEP_SLEEP;
}
