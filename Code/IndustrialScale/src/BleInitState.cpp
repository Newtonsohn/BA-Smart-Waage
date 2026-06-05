#include "BleInitState.h"
#include "Properties.h"
#include "DebugToggle.h"
#include <NimBLEDevice.h>

#define WAKEUP_BY_TIMER      0x00
#define WAKEUP_BY_POWER_CYCLE 0x01

#define WAKE_UP_FLAG_SIZE 1
#define BLE_ADV_FLAGS     0x06
#define MAC_ADDRESS_LENGTH 6
#define COMPANY_ID_LOW    0xFF
#define COMPANY_ID_HIGH   0xFF

// Persists across deep sleeps, resets on power cycle — used to detect missed advertisements.
// uint16 (max 65535) saves 2 bytes vs uint32, keeping the advertisement packet at exactly 31 bytes.
RTC_DATA_ATTR static uint16_t s_advertisementCounter = 0;

// ----------------------------- Callbacks -----------------------------

class ServerCallbacks : public NimBLEServerCallbacks {
  void onConnect(NimBLEServer* bleServer) override {
    Logger::log("Device connected.");
    Properties::deviceConnected = true;
  }
  void onDisconnect(NimBLEServer* bleServer) override {
    Logger::log("Device disconnected.");
    Properties::deviceConnected = false;
  }
};

class ConfigCharacteristicCallbacks : public NimBLECharacteristicCallbacks {
  void onRead(NimBLECharacteristic* pChar) override {
    Logger::log("Read request from gateway");
    if (pChar->getUUID() == NimBLEUUID(Properties::CHARACTERISTIC_CONFIG_UUID)) {
      uint8_t wakeupFlag = Properties::wakeUpCauseIsTimer ? WAKEUP_BY_TIMER : WAKEUP_BY_POWER_CYCLE;
      pChar->setValue(&wakeupFlag, WAKE_UP_FLAG_SIZE);
    }
  }

  void onWrite(NimBLECharacteristic* pChar) override {
    Logger::log(("Write request from Gateway: " + pChar->getUUID().toString()).c_str());
    if (pChar->getUUID() == NimBLEUUID(Properties::CHARACTERISTIC_CONFIG_UUID)) {
      Properties::configurationString += pChar->getValue().c_str();
      Logger::log(Properties::configurationString.c_str());
      if (Properties::configurationString.indexOf('}') == -1) {
        return;
      }
      Logger::log("Configuration received from Gateway");
      Properties::configurationReceived = true;
    }
  }
};

// In NimBLE, CCCD is handled automatically — subscription state tracked via onSubscribe on the characteristic.
class MeasureCharacteristicCallbacks : public NimBLECharacteristicCallbacks {
  void onSubscribe(NimBLECharacteristic* pChar, ble_gap_conn_desc* desc, uint16_t subValue) override {
    if (subValue == 0x0002) {
      Logger::log("Gateway subscribed to indications.");
      Properties::gwSubscribed = true;
    } else if (subValue == 0x0001) {
      Logger::log("Gateway subscribed to notifications.");
      Properties::gwSubscribed = true;
    } else {
      Logger::log("Gateway unsubscribed.");
      Properties::gwSubscribed = false;
    }
  }
};

// ----------------------------- Callbacks End -----------------------------

BleInitState::BleInitState() {}

void BleInitState::enter() {
  Logger::log("Load BleInit State");

  hw = HwContext::get();

  if (Properties::wakeUpCauseIsTimer) 
  debugToggle();  // MARKER 1: before BLE init
  NimBLEDevice::init("IndustryScale");

  memcpy(Properties::bleMacAddress, NimBLEDevice::getAddress().getNative(), MAC_ADDRESS_LENGTH);

  if (Properties::wakeUpCauseIsTimer) {
    // Timer wake-up: advertise weight + counter in manufacturer data, no connection needed
    s_advertisementCounter++;
    Logger::log(("Advertisement #" + String(s_advertisementCounter) + "  weight: " + String(Properties::currentWeight) + " g").c_str());

    // Payload layout (8 bytes; after BlueZ strips 2-byte company ID, gateway sees 6 bytes):
    //   bytes 0-3: float   currentWeight
    //   bytes 4-5: uint16  s_advertisementCounter
    // Packet budget: flags(3) + mfg_data(8+2=10) + UUID(18) = 31 bytes — exactly at the limit.
    uint8_t mfgPayload[8];
    mfgPayload[0] = COMPANY_ID_LOW;
    mfgPayload[1] = COMPANY_ID_HIGH;
    memcpy(&mfgPayload[2], &Properties::currentWeight, sizeof(float));
    memcpy(&mfgPayload[6], &s_advertisementCounter, sizeof(uint16_t));

    NimBLEAdvertisementData advData;
    advData.setFlags(BLE_ADV_FLAGS);
    advData.setManufacturerData(std::string((char*)mfgPayload, 8));
    advData.setCompleteServices(NimBLEUUID(Properties::SERVICE_UUID));

    hw->bleAdvertising = NimBLEDevice::getAdvertising();
    hw->bleAdvertising->setScanResponse(false);
    hw->bleAdvertising->setMinInterval(0x20);  // 20ms = minimum BLE advertising interval
    hw->bleAdvertising->setMaxInterval(0x20);
    hw->bleAdvertising->setAdvertisementData(advData);
    debugToggle();  // MARKER 2: after init, before advertising.start()
    hw->bleAdvertising->start();

    Logger::log("Started BLE advertisement with weight data");
  } else {
    // Power-cycle: full GATT server for configuration
    hw->bleServer = NimBLEDevice::createServer();
    hw->bleServer->setCallbacks(new ServerCallbacks());

    hw->bleService = hw->bleServer->createService(Properties::SERVICE_UUID);

    hw->bleConfigCharacteristic = hw->bleService->createCharacteristic(
      Properties::CHARACTERISTIC_CONFIG_UUID,
      NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);

    hw->bleMeasureCharacteristic = hw->bleService->createCharacteristic(
      Properties::CHARACTERISTIC_MEASURE_UUID,
      NIMBLE_PROPERTY::INDICATE);

    hw->bleConfigCharacteristic->setCallbacks(new ConfigCharacteristicCallbacks());
    hw->bleMeasureCharacteristic->setCallbacks(new MeasureCharacteristicCallbacks());

    hw->bleService->start();

    NimBLEAdvertisementData advData;
    advData.setCompleteServices(NimBLEUUID(Properties::SERVICE_UUID));
    advData.setFlags(BLE_ADV_FLAGS);
    advData.setName("IndustryScale");

    hw->bleAdvertising = NimBLEDevice::getAdvertising();
    hw->bleAdvertising->setScanResponse(false);
    hw->bleAdvertising->setAdvertisementData(advData);
    hw->bleAdvertising->start();

    Logger::log("Started BLE advertising (GATT server for configuration)");
  }
}

void BleInitState::update() {}

void BleInitState::exit() {
  Logger::log("BLE initialization completed");
  Logger::log("Exit BLE initialization State");
}

StateType BleInitState::nextState() {
  if (Properties::wakeUpCauseIsTimer) {
    return StateType::SEND_DATA;
  } else {
    return StateType::CONFIGURATION;
  }
}
