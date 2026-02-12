#include "SendDataState.h"
#include "Properties.h"
#include "StateType.h"

#define MILLISECONDS_PER_SECOND 1000
#define BROADCAST_DURATION_MS 300

// Company identifier in the manufacturer data field (2 bytes, little-endian).
// 0xFFFF is reserved for internal/testing use.
#define COMPANY_ID_LOW  0xFF
#define COMPANY_ID_HIGH 0xFF

SendDataState::SendDataState() {}

void SendDataState::enter() {
  Logger::log("Enter SendData State");

  hw = HwContext::get();

  startTime = millis();

  // Stop current advertising to update the payload
  hw->bleAdvertising->stop();

  // Build manufacturer data: 2-byte company ID + 4-byte float weight
  uint8_t weightBytes[sizeof(float)];
  memcpy(weightBytes, &Properties::currentWeight, sizeof(float));

  std::string mfgData;
  mfgData += (char)COMPANY_ID_LOW;
  mfgData += (char)COMPANY_ID_HIGH;
  mfgData += (char)weightBytes[0];
  mfgData += (char)weightBytes[1];
  mfgData += (char)weightBytes[2];
  mfgData += (char)weightBytes[3];

  // Set advertising data with manufacturer payload
  BLEAdvertisementData advData;
  advData.setFlags(0x06);  // General discoverable + BLE only
  advData.setManufacturerData(mfgData);

  hw->bleAdvertising->setAdvertisementData(advData);
  hw->bleAdvertising->start();

  broadcastStarted = true;

  Logger::log(("Broadcasting weight: " + std::to_string(Properties::currentWeight)).c_str());
}

void SendDataState::update() {
  if (!sendDataSuccess && broadcastStarted) {
    // Broadcast for BROADCAST_DURATION_MS then stop
    if (millis() - startTime >= BROADCAST_DURATION_MS) {
      hw->bleAdvertising->stop();
      sendDataSuccess = true;
      Properties::prevWeight = Properties::currentWeight;
      Logger::log("Broadcast complete.");
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
