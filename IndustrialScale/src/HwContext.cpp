#include "HwContext.h"
#include "Properties.h"
#include <Arduino.h>

std::shared_ptr<HwContext> HwContext::get() {
  static std::shared_ptr<HwContext> instance = std::make_shared<HwContext>();
  return instance;
}

int32_t HwContext::readADS1234() {
  // Wait for DRDY to go LOW (conversion ready)
  while (digitalRead(Properties::ADS1234_DRDY_DOUT) == HIGH) {
    delayMicroseconds(100);
  }

  // ADS1234 prefixes 2 channel-ID bits (A1, A0) before the 24-bit data.
  // With 24 clocks we capture [A1, A0, bit23..bit2]. Shift left 2 to align.
  // bit1 and bit0 become 0 — max error is 3 counts, negligible for 24-bit.
  uint32_t raw = 0;
  for (int i = 0; i < 24; i++) {
    digitalWrite(Properties::ADS1234_SCLK, HIGH);
    delayMicroseconds(2);
    raw = (raw << 1) | digitalRead(Properties::ADS1234_DRDY_DOUT);
    digitalWrite(Properties::ADS1234_SCLK, LOW);
    delayMicroseconds(2);
  }
  raw = (raw << 2) & 0x00FFFFFF;

  // Sign-extend 24-bit → 32-bit two's complement
  if (raw & 0x800000) raw |= 0xFF000000;
  return (int32_t)raw;
}

long HwContext::averageScaleReading() {
  long sum = 0;
  for (int i = 0; i < Properties::SCALE_READING_SAMPLE_COUNT; i++) {
    sum += readADS1234();
  }
  return sum / Properties::SCALE_READING_SAMPLE_COUNT;
}