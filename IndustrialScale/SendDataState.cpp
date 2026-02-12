#include "SendDataState.h"
#include "Properties.h"
#include "StateType.h"

#define MILLISECONDS_PER_SECOND 1000

SendDataState::SendDataState() {}

void SendDataState::enter() {
  Logger::log("Enter SendData State");

  hw = HwContext::get();

  startTime = millis();

  Logger::log("waiting for subscription...");
}

void SendDataState::update() {
  if (!sendDataSuccess) {
    if (Properties::gwSubscribed) {
      Logger::log("send data...");

      // Converts float pointer to byte pointer. Treats memory as sequence of bytes. Raw binary prepresentation of float is transmitted
      hw->bleMeasureCharacteristic->setValue(reinterpret_cast<uint8_t*>(&Properties::currentWeight), sizeof(Properties::currentWeight));
      // Sending data to subscriber with indication over BLE
      hw->bleMeasureCharacteristic->indicate();

      sendDataSuccess = true;

      Properties::prevWeight = Properties::currentWeight;
    }
  }
}

void SendDataState::exit() {
  Logger::log("Exit Send Data State");
}

StateType SendDataState::nextState() {
  unsigned long elapsedTime = millis() - startTime;
  if (elapsedTime >= Properties::connectionTimeout * MILLISECONDS_PER_SECOND) {
    handleFailure("Sending data timed out");

    Properties::sendDataFailed = true;

    return StateType::DISPLAY_INIT;
  }

  // Keep current state if data not yet sent successfull
  if (sendDataSuccess) {
    return StateType::DEEP_SLEEP;
  } else {
    return StateType::SEND_DATA;
  }
}

void SendDataState::handleFailure(const String& errorMessage) {
  Logger::log(errorMessage.c_str());
  strncpy(Properties::failureMessage, errorMessage.c_str(), sizeof(Properties::failureMessage));
}
