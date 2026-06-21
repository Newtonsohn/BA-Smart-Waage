#pragma once

#include <memory>
#include <NimBLEDevice.h>
#include <GxEPD2_BW.h>

using DisplayType = GxEPD2_BW<GxEPD2_290_BS, GxEPD2_290_BS::HEIGHT>;

class HwContext {
public:
  // Bluetooth
  NimBLEServer* bleServer = nullptr;
  NimBLEService* bleService = nullptr;
  NimBLECharacteristic* bleConfigCharacteristic = nullptr;
  NimBLECharacteristic* bleMeasureCharacteristic = nullptr;
  NimBLEAdvertising* bleAdvertising = nullptr;

  // Display
  std::shared_ptr<DisplayType> display;

  static std::shared_ptr<HwContext> get();

  // Low-level: select ADS1234 channel (1–4) via A0/A1 pins.
  void setChannel(int ch);

  // Read one sample from the currently selected channel.
  // Blocks until DRDY goes LOW. Includes 25th SCLK pulse to reset DRDY.
  int32_t readADS1234();

  // Switch to channel ch, then read one sample.
  // Applies PDWN workaround for CH4→CH1 transition where DRDY doesn't reset.
  int32_t readChannel(int ch);

    // Average n samples from channel ch with light sleep between samples.
  float readAverage(int ch, int n);

  // Average n samples from channel ch without entering light sleep.
  float readAverageNoSleep(int ch, int n);

  // Calibrated weight in grams: sum of spanFactor[i] * (avg_ch[i] - zeroOffset[i]).
  // Returns 0 if calibrationValid is false.
  float measureWeight();
};
