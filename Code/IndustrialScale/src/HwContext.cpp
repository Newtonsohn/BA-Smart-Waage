#include "HwContext.h"
#include "Properties.h"
#include <Arduino.h>
#include "esp_sleep.h"
#include "driver/gpio.h"

#define MEASURE_SAMPLES_PER_CHANNEL 5

std::shared_ptr<HwContext> HwContext::get() {
  static std::shared_ptr<HwContext> instance = std::make_shared<HwContext>();
  return instance;
}

void HwContext::setChannel(int ch) {
  int i = ch - 1;
  digitalWrite(Properties::ADS1234_A0, i & 0x01);
  digitalWrite(Properties::ADS1234_A1, (i >> 1) & 0x01);
}

int32_t HwContext::readADS1234() {
  while (digitalRead(Properties::ADS1234_DRDY_DOUT) == HIGH) {}

  uint32_t raw = 0;
  for (int i = 0; i < 24; i++) {
    digitalWrite(Properties::ADS1234_SCLK, HIGH);
    delayMicroseconds(2);
    raw = (raw << 1) | digitalRead(Properties::ADS1234_DRDY_DOUT);
    digitalWrite(Properties::ADS1234_SCLK, LOW);
    delayMicroseconds(2);
  }
  // 25th clock: forces DRDY HIGH so the next channel switch gets a clean DRDY edge
  digitalWrite(Properties::ADS1234_SCLK, HIGH); delayMicroseconds(2);
  digitalWrite(Properties::ADS1234_SCLK, LOW);  delayMicroseconds(2);

  if (raw & 0x800000) raw |= 0xFF000000;
  return (int32_t)raw;
}

int32_t HwContext::readChannel(int ch) {
  setChannel(ch);
  // CH4(A1=1,A0=1) → CH1(A1=0,A0=0): no rising edge on A0/A1, so ADS1234 never
  // raises DRDY. Toggle PDWN to force a fresh conversion cycle.
  if (ch == 1 && digitalRead(Properties::ADS1234_DRDY_DOUT) == LOW) {
    digitalWrite(Properties::ADS1234_PDWN, LOW);
    delayMicroseconds(500);
    digitalWrite(Properties::ADS1234_PDWN, HIGH);
  }
  return readADS1234();
}

float HwContext::readAverage(int ch, int n) {
  gpio_wakeup_enable((gpio_num_t)Properties::ADS1234_DRDY_DOUT, GPIO_INTR_LOW_LEVEL);
  esp_sleep_enable_gpio_wakeup();

  setChannel(ch);

  long long sum = 0;
  for (int i = 0; i < n; i++) {
    if (digitalRead(Properties::ADS1234_DRDY_DOUT) == HIGH) {
      esp_light_sleep_start();
    }
    sum += readADS1234();
  }

  gpio_wakeup_disable((gpio_num_t)Properties::ADS1234_DRDY_DOUT);
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_GPIO);

  return (float)sum / n;
}

float HwContext::measureWeight() {
  if (!Properties::calibrationValid) return 0.0f;

  float weight = 0;
  for (int ch = 1; ch <= 4; ch++) {
    float avg = readAverage(ch, MEASURE_SAMPLES_PER_CHANNEL);
    weight += Properties::spanFactor[ch - 1] * (avg - Properties::zeroOffset[ch - 1]);
  }
  return weight;
}
