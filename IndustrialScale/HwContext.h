#pragma once

#include <memory>
#include <BLEServer.h>
#include <BLECharacteristic.h>
#include <GxEPD2_BW.h>
#include <HX711.h>

// Define the display type once for reuse.
using DisplayType = GxEPD2_BW<GxEPD2_290_BS, GxEPD2_290_BS::HEIGHT>;

/**
 * @class HwContext
 * @brief Harware context that holds sahred pointers to all hardware components.
 *
 * Class servers as singleton-like container for all hardware components (BLE, Display, Scale / ADC) 
 * used by the states of the state machine.
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

  // Scale / ADC
  std::shared_ptr<HX711> scale;

  // Get the singleton instance
  static std::shared_ptr<HwContext> get();

  // Average scale readings
  long averageScaleReading();
};
