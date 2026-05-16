#include "Arduino.h"
#include "ConfigurationState.h"
#include "Properties.h"
#include <NimBLEDevice.h>
#include <ArduinoJson.h>

#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>

#define JSON_BUFFER_SIZE 512

#define DISPLAY_X_MARGIN 10
#define DISPLAY_Y_TITLE 30
#define DISPLAY_Y_MAC 60
#define DISPLAY_Y_WEIGHT 90
#define DISPLAY_Y_ITEM_NAME 30
#define DISPLAY_Y_ITEM_NUMBER 55
#define DISPLAY_Y_DEVICE_NAME 110
#define DISPLAY_Y_STATUS 110
#define DISPLAY_X_STATUS 170


#define MILLISECONDS_PER_SECOND 1000
#define DEFAULT_HEARTBEAT_TRIGGER 0

ConfigurationState::ConfigurationState() {}

void ConfigurationState::enter() {
  Logger::log("Load Configuration State");

  startTime = millis();

  hw = HwContext::get();

  snprintf(cachedMac, sizeof(cachedMac), "%02X:%02X:%02X:%02X:%02X:%02X",
    Properties::bleMacAddress[0], Properties::bleMacAddress[1],
    Properties::bleMacAddress[2], Properties::bleMacAddress[3],
    Properties::bleMacAddress[4], Properties::bleMacAddress[5]);

  Logger::log(("BLE MAC: " + String(cachedMac)).c_str());
  Logger::log("waiting for configuration...");
}

void ConfigurationState::update() {
  if (!configurationSuccess) {
    if (millis() - lastWeightUpdateMs >= 3000) {
      lastWeightUpdateMs = millis();

      float weight = 0;
      if (Properties::calibrationValid) {
        for (int ch = 1; ch <= 4; ch++) {
          int32_t raw = hw->readChannel(ch);
          weight += Properties::spanFactor[ch - 1] * (raw - Properties::zeroOffset[ch - 1]);
        }
      }
      char weightBuf[20];
      snprintf(weightBuf, sizeof(weightBuf), "%.1f g", weight);
      Logger::log(("Weight: " + String(weightBuf)).c_str());

      hw->display->fillScreen(GxEPD_WHITE);
      hw->display->setCursor(DISPLAY_X_MARGIN, DISPLAY_Y_TITLE);
      hw->display->setFont(&FreeSansBold12pt7b);
      hw->display->print("Waiting for GW");
      hw->display->setCursor(DISPLAY_X_MARGIN, DISPLAY_Y_MAC);
      hw->display->setFont(&FreeSans9pt7b);
      hw->display->print(cachedMac);
      hw->display->setCursor(DISPLAY_X_MARGIN, DISPLAY_Y_WEIGHT);
      hw->display->print(weightBuf);
      hw->display->display(true);
    }

    if (Properties::configurationReceived) {
      bool parseConfigurationSuccess = parseConfiguration();
      if (parseConfigurationSuccess) {
        updateDisplayWithConfiguration();
        configurationSuccess = true;
        Properties::saveConfigToNVS();
      }
    }
  }
}

void ConfigurationState::exit() {
  Logger::log("Exit Configuration State");
}

void ConfigurationState::updateDisplayWithConfiguration() {
  // Display on screen
  hw->display->fillScreen(GxEPD_WHITE);

  // Product item name
  hw->display->setCursor(DISPLAY_X_MARGIN, DISPLAY_Y_ITEM_NAME);
  hw->display->setFont(&FreeSansBold12pt7b);
  hw->display->print(Properties::itemName);

  // Product item number
  hw->display->setCursor(DISPLAY_X_MARGIN, DISPLAY_Y_ITEM_NUMBER);
  hw->display->setFont(&FreeSans9pt7b);
  hw->display->print(Properties::itemNumber);

  // Device name
  hw->display->setFont(&FreeSans9pt7b);
  hw->display->setCursor(DISPLAY_X_MARGIN, DISPLAY_Y_DEVICE_NAME);
  hw->display->print(Properties::deviceName);

  // Status
  hw->display->setCursor(DISPLAY_X_STATUS, DISPLAY_Y_STATUS);
  hw->display->print("Gw Connected");

  hw->display->display(true);
}

bool ConfigurationState::parseConfiguration() {
  // Parse the accumulated buffer
  StaticJsonDocument<JSON_BUFFER_SIZE> doc;
  DeserializationError error = deserializeJson(doc, Properties::configurationString);

  if (error) {
    handleFailure("Failed to parse JSON: " + String(error.c_str()));
    return false;
  }

  // Store the parsed device properties
  strncpy(Properties::deviceName, doc["name"] | "", sizeof(Properties::deviceName));
  Properties::updateInterval = doc["updateInterval"];
  if (Properties::updateInterval > 0) {
    Properties::heartbeatTrigger = doc["heartBeatInterval"].as<int>() / doc["updateInterval"].as<int>();
  } else {
    Properties::heartbeatTrigger = DEFAULT_HEARTBEAT_TRIGGER;
  }

  // Store the parsed item properties
  strncpy(Properties::itemName, doc["inventoryItem"]["itemName"] | "", sizeof(Properties::itemNumber));
  strncpy(Properties::itemNumber, doc["inventoryItem"]["itemNumber"] | "", sizeof(Properties::itemNumber));
  Properties::itemWeight = doc["inventoryItem"]["itemWeight"];

  return true;
}

void ConfigurationState::handleFailure(const String& errorMessage) {
  Logger::log(errorMessage.c_str());
  strncpy(Properties::failureMessage, errorMessage.c_str(), sizeof(Properties::failureMessage));
}

StateType ConfigurationState::nextState() {
  unsigned long elapsedTime = millis() - startTime;
  if (elapsedTime >= Properties::connectionTimeout * MILLISECONDS_PER_SECOND) {
    handleFailure("Configuration timed out");
    return StateType::FAILURE;
  }

  // Keep current state if configuration not yet successfull
  if (configurationSuccess) {
    return StateType::DEEP_SLEEP;
  } else {
    return StateType::CONFIGURATION;
  }
}
