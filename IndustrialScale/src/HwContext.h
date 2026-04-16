#pragma once

#include <memory>
#include <BLEServer.h>
#include <BLECharacteristic.h>
#include <GxEPD2_BW.h>

// Define the display type once for reuse.
using DisplayType = GxEPD2_BW<GxEPD2_290_BS, GxEPD2_290_BS::HEIGHT>;

/**
 * @class HwContext
 * @brief Hardware context that holds shared pointers to all hardware components.
 *
 * Singleton-like container for BLE, Display and ADS1234 ADC access.
 */
class HwContext {
public:
  // Bluetooth
  BLEServer* bleServer = nullptr;
  BLEService* bleService = nullptr;
  BLECharacteristic* bleConfigCharacteristic = nullptr;
  BLECharacteristic* bleMeasureCharacteristic = nullptr;
  BLEAdvertising* bleAdvertising = nullptr;

  // Display
  std::shared_ptr<DisplayType> display;

  // Get the singleton instance
  static std::shared_ptr<HwContext> get();

  // Read one 24-bit two's-complement sample from the ADS1234.
  // Blocks until DRDY goes LOW (conversion ready).
  int32_t readADS1234();

  // Average multiple ADS1234 readings.
  long averageScaleReading();
};
