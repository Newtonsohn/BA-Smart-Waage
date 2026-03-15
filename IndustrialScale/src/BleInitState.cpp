#include "BleInitState.h"
#include "Properties.h"
#include <BLEDevice.h>
#include <BLE2902.h>

#define WAKEUP_BY_TIMER 0x00
#define WAKEUP_BY_POWER_CYCLE 0x01

#define BLE_CCCD_DISABLED 0x00
#define BLE_CCCD_NOTIFICATION_ENABLED 0x01
#define BLE_CCCD_INDICATION_ENABLED 0x02
#define WAKE_UP_FLAG_SIZE 1

#define BLE_ADV_FLAGS 0x06
#define BLE_CONN_PARAMS 0x00

#define MAC_ADDRESS_LENGTH 6

// ----------------------------- Callbacks -----------------------------
// Callback-Klasse for BLE-Server
class ServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* bleServer) {
    Logger::log("Device connected.");
    Properties::deviceConnected = true;
  }

  void onDisconnect(BLEServer* bleServer) {
    Logger::log("Device disconnected.");
    Properties::deviceConnected = false;
  }
};

// Callback class for onfig characteristics
class ConfigCharacteristicCallbacks : public BLECharacteristicCallbacks {
  String jsonBuffer;

  void onRead(BLECharacteristic* bleCharacteristic) override {
    Logger::log("Read request from gateway");
    if (bleCharacteristic->getUUID().equals(BLEUUID(Properties::CHARACTERISTIC_CONFIG_UUID))) {
      // Get Config, when there was a power cycle (not timer)
      uint8_t wakeupFlag;
      if (Properties::wakeUpCauseIsTimer) {
        wakeupFlag = WAKEUP_BY_TIMER;
      } else {
        wakeupFlag = WAKEUP_BY_POWER_CYCLE;
      }
      bleCharacteristic->setValue(&wakeupFlag, WAKE_UP_FLAG_SIZE);
    }
  }

  void onWrite(BLECharacteristic* bleCaracteristic) override {
    Logger::log(("Write request from Gateway: " + bleCaracteristic->getUUID().toString()).c_str());
    if (bleCaracteristic->getUUID().equals(BLEUUID(Properties::CHARACTERISTIC_CONFIG_UUID))) {
      Properties::configurationString += bleCaracteristic->getValue().c_str();
      Logger::log(Properties::configurationString.c_str());
      if (Properties::configurationString.indexOf('}') == -1) {
        // Waiting for end of JSON file
        return;
      }
      Logger::log("Configuration received from Gateway");
      Properties::configurationReceived = true;
    }
  }
};

// Callback class for measure characteristics
class MeasureCharacteristicCallbacks : public BLEDescriptorCallbacks {
  void onWrite(BLEDescriptor* descriptor) override {
    const uint8_t* val = descriptor->getValue();
    if (BLE_CCCD_INDICATION_ENABLED == val[0]) {  // Indication enabled
      Logger::log("Gateway subscribed to indications.");
      Properties::gwSubscribed = true;
    } else if (BLE_CCCD_NOTIFICATION_ENABLED == val[0]) {  // Notification enabled
      Logger::log("Gateway subscribed to notifications.");
      Properties::gwSubscribed = true;
    } else if (BLE_CCCD_DISABLED == val[0]) {  // Notification and indication disabled
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

  // Init BLE
  BLEDevice::init("IndustryScale");

  // Get the Bluetooth MAC address
  memcpy(Properties::bleMacAddress, BLEDevice::getAddress().getNative(), MAC_ADDRESS_LENGTH);

  // Create BLE-Server
  hw->bleServer = BLEDevice::createServer();
  hw->bleServer->setCallbacks(new ServerCallbacks());

  // Create BLE-Service
  hw->bleService = hw->bleServer->createService(Properties::SERVICE_UUID);

  // Create configuration characteristics
  hw->bleConfigCharacteristic = hw->bleService->createCharacteristic(
    Properties::CHARACTERISTIC_CONFIG_UUID,
    BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);

  // Create measure characteristics
  hw->bleMeasureCharacteristic = hw->bleService->createCharacteristic(
    Properties::CHARACTERISTIC_MEASURE_UUID,
    BLECharacteristic::PROPERTY_INDICATE);

  // Register callbacks
  hw->bleConfigCharacteristic->setCallbacks(new ConfigCharacteristicCallbacks());

  // Add Descriptor -> Required to check if gateway subscribed
  BLE2902* cccd = new BLE2902();
  cccd->setCallbacks(new MeasureCharacteristicCallbacks());
  hw->bleMeasureCharacteristic->addDescriptor(cccd);

  // Start service
  hw->bleService->start();

  // Prepare advertising
  BLEAdvertisementData advData;
  advData.setCompleteServices(BLEUUID(Properties::SERVICE_UUID));
  advData.setFlags(BLE_ADV_FLAGS);  // BLE General discoverable mode (bit 1: 0x02) and BLE only (bit 2: 0x04)
  advData.setName("IndustryScale");

  hw->bleAdvertising = BLEDevice::getAdvertising();
  hw->bleAdvertising->addServiceUUID(Properties::SERVICE_UUID);
  hw->bleAdvertising->setScanResponse(false);
  hw->bleAdvertising->setAdvertisementData(advData);
  hw->bleAdvertising->setMinPreferred(BLE_CONN_PARAMS);  // All parameters disabled
  hw->bleAdvertising->setMaxPreferred(BLE_CONN_PARAMS);  // All parameters disabled
  hw->bleAdvertising->start();

  Logger::log("Started BLE advertising");
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
