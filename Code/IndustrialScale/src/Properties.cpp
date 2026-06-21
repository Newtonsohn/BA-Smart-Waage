#include "Properties.h"
#include "nvs.h"

#define MAX_STRING_LENGTH_64 64
#define MAC_ADDRESS_LENGTH 6

namespace Properties {
// ADS1234 Pins
const int ADS1234_DRDY_DOUT = 14;   // DRDY + DOUT combined, active low, 32 -> alt IO2
const int ADS1234_SCLK      = 15;
const int ADS1234_PDWN      = 26;  // active low: LOW=power-down, HIGH=run
const int ADS1234_DMS_PWR   = 17;  // MOSFET DMS power, active low
const int ADS1234_A0        = 4;  // channel select bit 0, 4 -> alt IO32
const int ADS1234_A1        = 2;   // channel select bit 1, 2 ->alt IO4

// Display Pins
const int DISPLAY_CS_PIN    = 32;   // Blau 14, aber 14 wird jetzt von drdy/dout genutzt, also 32 -> alt IO14
const int DISPLAY_DC_PIN    = 5;   // Weiss 5
const int DISPLAY_RESET_PIN = 19;   // Orange 19
const int DISPLAY_BUSY_PIN  = 16;   // Violett 16
// SCL = 18 //Grün
// SDA = 23 //Gelb

// Settings
int connectionTimeout = 1800;
int tareTime = 10;
bool loggingEnabled = true;

const char* BACKEND_URL = "https://192.168.2.100:7093/bins/integration?mac=";

// BLE
const char* SERVICE_UUID = "f0cbd08a-41a1-4b14-b66f-420e6c7f6d1f";
const char* CHARACTERISTIC_CONFIG_UUID = "b4520837-7d4f-4e4a-96ff-8f9cd9e64577";
const char* CHARACTERISTIC_MEASURE_UUID = "a03cfc2e-370e-4c34-9d8e-9d75f6e93e88";
RTC_DATA_ATTR uint8_t bleMacAddress[MAC_ADDRESS_LENGTH] = { 0, 0, 0, 0, 0, 0 };

// Scale
RTC_DATA_ATTR float zeroOffset[4]  = {0, 0, 0, 0};
RTC_DATA_ATTR float spanFactor[4]  = {0, 0, 0, 0};
RTC_DATA_ATTR bool  calibrationValid = false;
RTC_DATA_ATTR float currentWeight  = 0.0f;
RTC_DATA_ATTR float prevWeight     = 0.0f;

// Configuration
char deviceName[MAX_STRING_LENGTH_64] = "SmartScale";
char itemName[MAX_STRING_LENGTH_64]   = "No Product";
char itemNumber[MAX_STRING_LENGTH_64] = "No Product";
RTC_DATA_ATTR int heartbeatTrigger = 0;
RTC_DATA_ATTR int updateInterval   = 300;
RTC_DATA_ATTR float itemWeight     = 5000.0f;

// States
bool deviceConnected      = false;
bool gwSubscribed         = false;
bool configurationReceived = false;
String configurationString = "";
bool sendDataFailed       = false;
RTC_DATA_ATTR int heartbeatCounter = 0;
bool wakeUpCauseIsTimer   = false;
char failureMessage[MAX_STRING_LENGTH_64] = "";

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
  nvs_set_blob(nvs_handle, "prevWeight", &prevWeight, sizeof(prevWeight));

  // 4-channel calibration data
  nvs_set_blob(nvs_handle, "zeroOffset", zeroOffset, sizeof(zeroOffset));
  nvs_set_blob(nvs_handle, "spanFactor", spanFactor, sizeof(spanFactor));
  uint8_t calByte = calibrationValid ? 0xAB : 0x00;
  nvs_set_u8(nvs_handle, "calValid", calByte);

  err = nvs_commit(nvs_handle);
  nvs_close(nvs_handle);
  return err;
}

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

  nvs_get_i32(nvs_handle, "heartbeat",  reinterpret_cast<int32_t*>(&heartbeatTrigger));
  nvs_get_i32(nvs_handle, "heartbeatC", reinterpret_cast<int32_t*>(&heartbeatCounter));
  nvs_get_i32(nvs_handle, "updateIntv", reinterpret_cast<int32_t*>(&updateInterval));

  size_t sizeOfMac = 6;
  nvs_get_blob(nvs_handle, "ble_mac", bleMacAddress, &sizeOfMac);

  size_t sizeOfFloat = sizeof(float);
  nvs_get_blob(nvs_handle, "itemWeight", &itemWeight, &sizeOfFloat);
  nvs_get_blob(nvs_handle, "prevWeight", &prevWeight, &sizeOfFloat);

  // 4-channel calibration data
  size_t sizeOf4Floats = sizeof(zeroOffset);
  nvs_get_blob(nvs_handle, "zeroOffset", zeroOffset, &sizeOf4Floats);
  sizeOf4Floats = sizeof(spanFactor);
  nvs_get_blob(nvs_handle, "spanFactor", spanFactor, &sizeOf4Floats);
  uint8_t calByte = 0x00;
  nvs_get_u8(nvs_handle, "calValid", &calByte);
  calibrationValid = (calByte == 0xAB);

  if (updateInterval <= 0) updateInterval = 300;

  nvs_close(nvs_handle);
  return ESP_OK;
}
}
