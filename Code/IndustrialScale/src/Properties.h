#pragma once

#include <Arduino.h>
#include "esp_err.h"

#define MAX_STRING_LENGTH_64 64
#define MAC_ADDRESS_LENGTH 6

namespace Properties {
// ADS1234 Pins
extern const int ADS1234_DRDY_DOUT;  // DRDY + DOUT combined, active low
extern const int ADS1234_SCLK;
extern const int ADS1234_PDWN;       // active low: LOW=power-down, HIGH=run
extern const int ADS1234_DMS_PWR;    // MOSFET DMS power, active low
extern const int ADS1234_A0;         // channel select bit 0
extern const int ADS1234_A1;         // channel select bit 1

// Display Pins
extern const int DISPLAY_CS_PIN;
extern const int DISPLAY_DC_PIN;
extern const int DISPLAY_RESET_PIN;
extern const int DISPLAY_BUSY_PIN;

// Settings
extern int connectionTimeout;
extern int tareTime;
extern bool loggingEnabled;
extern const char* BACKEND_URL;

// BLE
extern const char* SERVICE_UUID;
extern const char* CHARACTERISTIC_CONFIG_UUID;
extern const char* CHARACTERISTIC_MEASURE_UUID;
extern uint8_t bleMacAddress[MAC_ADDRESS_LENGTH];

// Scale — 4-channel corner-load compensation
extern float zeroOffset[4];   // raw ADC zero per channel
extern float spanFactor[4];   // g/count per channel (encodes polarity)
extern bool  calibrationValid;
extern float currentWeight;
extern float prevWeight;

// Configuration
extern char deviceName[MAX_STRING_LENGTH_64];
extern char itemName[MAX_STRING_LENGTH_64];
extern char itemNumber[MAX_STRING_LENGTH_64];
extern int heartbeatTrigger;
extern int updateInterval;
extern float itemWeight;

// States
extern bool deviceConnected;
extern bool gwSubscribed;
extern bool configurationReceived;
extern String configurationString;
extern bool sendDataFailed;
extern int heartbeatCounter;
extern bool wakeUpCauseIsTimer;
extern char failureMessage[MAX_STRING_LENGTH_64];

extern esp_err_t saveConfigToNVS();
extern esp_err_t loadConfigFromNVS();
}
