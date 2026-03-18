#include "SendDataState.h"
#include "Properties.h"
#include "StateType.h"
#include <BLEDevice.h>

#define MILLISECONDS_PER_SECOND 1000

SendDataState::SendDataState() {}

void SendDataState::enter() {
  Logger::log("Enter SendData State");

  hw = HwContext::get();

  startTime = millis();

  if (Properties::wakeUpCauseIsTimer) {
    Logger::log("Advertisement mode: broadcasting weight, waiting for timeout...");
  } else {
    Logger::log("GATT mode: waiting for gateway subscription...");
  }
}

void SendDataState::update() {
  if (Properties::wakeUpCauseIsTimer) {
    delay(500);  // Nothing to do, just yield while advertising
    return;
  }

  if (!sendDataSuccess) {
    if (Properties::gwSubscribed) {
      Logger::log("send data...");

      hw->bleMeasureCharacteristic->setValue(reinterpret_cast<uint8_t*>(&Properties::currentWeight), sizeof(Properties::currentWeight));
      hw->bleMeasureCharacteristic->indicate();

      sendDataSuccess = true;

      Properties::prevWeight = Properties::currentWeight;
    }
  }
}

void SendDataState::exit() {
  Logger::log("Exit Send Data State");
  if (Properties::wakeUpCauseIsTimer) {
    BLEDevice::getAdvertising()->stop();
    BLEDevice::deinit(true);
  }
}

StateType SendDataState::nextState() {
  unsigned long elapsedTime = millis() - startTime;

  if (Properties::wakeUpCauseIsTimer) {
    // Advertise for a short window so the gateway has time to scan and receive the
    // advertisement, then go straight to deep sleep. No connection needed.
    if (elapsedTime >= 0.3 * MILLISECONDS_PER_SECOND) {
      return StateType::DEEP_SLEEP;
    }
    return StateType::SEND_DATA;
  }

  if (elapsedTime >= Properties::connectionTimeout * MILLISECONDS_PER_SECOND) {
    handleFailure("Sending data timed out");
    Properties::sendDataFailed = true;
    return StateType::DISPLAY_INIT;
  }

  if (sendDataSuccess) {
    return StateType::DEEP_SLEEP;
  }
  return StateType::SEND_DATA;
}

void SendDataState::handleFailure(const String& errorMessage) {
  Logger::log(errorMessage.c_str());
  strncpy(Properties::failureMessage, errorMessage.c_str(), sizeof(Properties::failureMessage));
}
