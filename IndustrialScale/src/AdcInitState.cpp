#include "AdcInitState.h"
#include "Properties.h"
#include "StateType.h"
#include <Arduino.h>

#define DRDY_POLL_DELAY_MS    10
#define DRDY_TIMEOUT_MS       3000

AdcInitState::AdcInitState() {}

void AdcInitState::enter() {
  Logger::log("Enter ADC initialization State");

  hw = HwContext::get();

  // Configure ADS1234 GPIO pins
  pinMode(Properties::ADS1234_SCLK,      OUTPUT);
  digitalWrite(Properties::ADS1234_SCLK, LOW);

  pinMode(Properties::ADS1234_DMS_PWR,   OUTPUT);
  digitalWrite(Properties::ADS1234_DMS_PWR, HIGH);  // MOSFET off initially

  pinMode(Properties::ADS1234_PDWN,      OUTPUT);
  digitalWrite(Properties::ADS1234_PDWN, HIGH);     // running state initially

  pinMode(Properties::ADS1234_DRDY_DOUT, INPUT);

  // Power-on sequence (datasheet Section 7.4.5):
  // Enable load cell power, then toggle PDWN low→high to start conversions
  digitalWrite(Properties::ADS1234_DMS_PWR, LOW);   // enable DMS power (active low)
  delay(100);
  digitalWrite(Properties::ADS1234_PDWN, LOW);      // assert power-down briefly
  delay(10);
  digitalWrite(Properties::ADS1234_PDWN, HIGH);     // release → chip starts converting
}

void AdcInitState::update() {
  // Wait for first DRDY falling edge (first conversion ready)
  if (digitalRead(Properties::ADS1234_DRDY_DOUT) == LOW) {
    Logger::log("ADC ready");
    adcIsReady = true;
  } else {
    delay(DRDY_POLL_DELAY_MS);
  }
}

void AdcInitState::exit() {
  Logger::log("Exit ADC initialization State");
}

StateType AdcInitState::nextState() {
  // Keep current state until ADC is ready
  if (!adcIsReady) {
    return StateType::ADC_INIT;
  }

  if (Properties::wakeUpCauseIsTimer) {
    return StateType::MEASURE;
  } else {
    return StateType::CALIBRATION_CHECK;
  }
}
