#include "Properties.h"
#include "nvs.h"

#define MAX_STRING_LENGTH_64 64
#define MAC_ADDRESS_LENGTH 6

namespace Properties {
// HX711 Pins
const int ADC_DT = 26;
const int ADC_SCK = 22;

// Display Pins
const int DISPLAY_CS_PIN = 5;
const int DISPLAY_DC_PIN = 16;
const int DISPLAY_RESET_PIN = 17;
const int DISPLAY_BUSY_PIN = 4;

// Settings
int connectionTimeout = 1800;  //Connection timeout in seconds
int tareTime = 1;           //Tare time in seconds
bool loggingEnabled = true;

const char* BACKEND_URL = "https://192.168.2.100:7093/bins/integration?mac=";

// BLE
const char* SERVICE_UUID = "f0cbd08a-41a1-4b14-b66f-420e6c7f6d1f";
const char* CHARACTERISTIC_CONFIG_UUID = "b4520837-7d4f-4e4a-96ff-8f9cd9e64577";
const char* CHARACTERISTIC_MEASURE_UUID = "a03cfc2e-370e-4c34-9d8e-9d75f6e93e88";
uint8_t bleMacAddress[MAC_ADDRESS_LENGTH] = { 0, 0, 0, 0, 0, 0 };

// Scale
const int SCALE_READING_SAMPLE_COUNT = 20;
const int SCALE_READING_SAMPLE_DELAY = 5;
float zeroOffset = 0;
float calFactor = 1;
float currentWeight = 0.0f;  //Current weight in grams
float prevWeight = 0.0f;     //Previous weight in grams

// Configuration
char deviceName[MAX_STRING_LENGTH_64] = "SmartScale";
char itemName[MAX_STRING_LENGTH_64] = "No Product";
char itemNumber[MAX_STRING_LENGTH_64] = "No Product";
int heartbeatTrigger = 0;
int updateInterval = 300;  //Update interval in seconds
float itemWeight = 5000.0f;

// States
bool deviceConnected = false;
bool gwSubscribed = false;
bool configurationReceived = false;
String configurationString = "";
bool sendDataFailed = false;
int heartbeatCounter = 0;
bool wakeUpCauseIsTimer = false;
char failureMessage[MAX_STRING_LENGTH_64] = "";

//Save to Non Volatile Storage - needed for deep sleep
esp_err_t saveConfigToNVS() {
  nvs_handle_t nvs_handle;
  esp_err_t err = nvs_open("storage", NVS_READWRITE, &nvs_handle);
  if (err != ESP_OK) return err;

  nvs_set_str(nvs_handle, "deviceName", deviceName);
  nvs_set_str(nvs_handle, "itemName", itemName);
  nvs_set_str(nvs_handle, "itemNumber", itemNumber);

  nvs_set_i32(nvs_handle, "heartbeat", heartbeatTrigger);
  nvs_set_i32(nvs_handle, "heartbeatC", heartbeatCounter);
  nvs_set_i32(nvs_handle, "updateIntv", updateInterval);

  nvs_set_blob(nvs_handle, "ble_mac", bleMacAddress, 6);

  nvs_set_blob(nvs_handle, "itemWeight", &itemWeight, sizeof(itemWeight));
  nvs_set_blob(nvs_handle, "zeroOffset", &zeroOffset, sizeof(zeroOffset));
  nvs_set_blob(nvs_handle, "calFactor", &calFactor, sizeof(calFactor));
  nvs_set_blob(nvs_handle, "prevWeight", &prevWeight, sizeof(prevWeight));

  err = nvs_commit(nvs_handle);
  nvs_close(nvs_handle);

  return err;
}

//Load from Non Volatile Storage - needed for deep sleep
esp_err_t loadConfigFromNVS() {
  nvs_handle_t nvs_handle;
  esp_err_t err = nvs_open("storage", NVS_READONLY, &nvs_handle);
  if (err != ESP_OK) return err;

  size_t len = sizeof(deviceName);
  nvs_get_str(nvs_handle, "deviceName", deviceName, &len);
  len = sizeof(itemName);
  nvs_get_str(nvs_handle, "itemName", itemName, &len);
  len = sizeof(itemNumber);
  nvs_get_str(nvs_handle, "itemNumber", itemNumber, &len);

  nvs_get_i32(nvs_handle, "heartbeat", reinterpret_cast<int32_t*>(&heartbeatTrigger));
  nvs_get_i32(nvs_handle, "heartbeatC", reinterpret_cast<int32_t*>(&heartbeatCounter));
  nvs_get_i32(nvs_handle, "updateIntv", reinterpret_cast<int32_t*>(&updateInterval));

  size_t sizeOfMacAddress = 6;
  nvs_get_blob(nvs_handle, "ble_mac", bleMacAddress, &sizeOfMacAddress);

  size_t sizeOfFloat = sizeof(float);
  nvs_get_blob(nvs_handle, "itemWeight", &itemWeight, &sizeOfFloat);
  nvs_get_blob(nvs_handle, "zeroOffset", &zeroOffset, &sizeOfFloat);
  nvs_get_blob(nvs_handle, "calFactor", &calFactor, &sizeOfFloat);
  nvs_get_blob(nvs_handle, "prevWeight", &prevWeight, &sizeOfFloat);

  nvs_close(nvs_handle);

  return ESP_OK;
}
}
