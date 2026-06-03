#include "CalibrationState.h"
#include "Properties.h"
#include <cmath>
#include <cstdio>
#include <Arduino.h>

#define CAL_SAMPLES      10       // averaged samples per channel per step
#define CAL_WEIGHT_G     3000.0f  // center calibration weight in grams
#define CORNER_WEIGHT_G  1000.0f  // corner trim weight in grams
#define MIN_DEFLECTION   10000.0f // minimum total deflection to accept cal

CalibrationState::CalibrationState() {}

void CalibrationState::enter() {
  Logger::log("Enter Calibration State");

  hw = HwContext::get();
  runCalibration();
}

void CalibrationState::update() {}

void CalibrationState::exit() {
  Logger::log("Exit Calibration State");
}

StateType CalibrationState::nextState() {
  return StateType::BLE_INIT;
}

// ── helpers ───────────────────────────────────────────────────────────────────

void CalibrationState::printAllChannels() {
  int32_t v[4];
  for (int ch = 1; ch <= 4; ch++) v[ch - 1] = hw->readChannel(ch);
  char buf[80];
  snprintf(buf, sizeof(buf), "ADC  CH1:%ld  CH2:%ld  CH3:%ld  CH4:%ld",
           (long)v[0], (long)v[1], (long)v[2], (long)v[3]);
  Logger::log(buf);
}

void CalibrationState::waitForSerial(const char* msg) {
  Logger::log(msg);
  while (Serial.available()) Serial.read();  // flush pending

  unsigned long lastPrint = 0;
  while (!Serial.available()) {
    if (millis() - lastPrint >= 1000) {
      lastPrint = millis();
      printAllChannels();
    }
    delay(50);
  }
  while (Serial.available()) Serial.read();  // consume
}

// ── calibration sequence ──────────────────────────────────────────────────────

void CalibrationState::runCalibration() {
  char buf[100];
  Logger::log("=== Calibration start ===");

  // ── Step 1: zero offset ───────────────────────────────────────────────────
  waitForSerial("[1/3] Remove ALL weight from the scale, then send any key.");

  Logger::log("Measuring zero offset...");
  for (int ch = 1; ch <= 4; ch++) {
    Properties::zeroOffset[ch - 1] = hw->readAverage(ch, CAL_SAMPLES);
    snprintf(buf, sizeof(buf), "  CH%d zero: %ld", ch, (long)Properties::zeroOffset[ch - 1]);
    Logger::log(buf);
  }

  // ── Step 2: centered span ─────────────────────────────────────────────────
  snprintf(buf, sizeof(buf),
           "[2/3] Place %.0fg weight CENTERED on scale, then send any key.", CAL_WEIGHT_G);
  waitForSerial(buf);

  Logger::log("Measuring centered span...");
  float deflection[4];
  float totalAbsDeflection = 0;
  for (int ch = 1; ch <= 4; ch++) {
    float val = hw->readAverage(ch, CAL_SAMPLES);
    deflection[ch - 1] = val - Properties::zeroOffset[ch - 1];
    totalAbsDeflection += fabsf(deflection[ch - 1]);
    int sign = (deflection[ch - 1] >= 0) ? 1 : -1;
    snprintf(buf, sizeof(buf), "  CH%d deflection: %+ld  [%s]",
             ch, (long)deflection[ch - 1], sign > 0 ? "normal" : "inverted");
    Logger::log(buf);
  }

  if (totalAbsDeflection < MIN_DEFLECTION) {
    Logger::log("ERROR: Deflection too small — check load cell wiring. Calibration aborted.");
    return;
  }

  float initScale = CAL_WEIGHT_G / totalAbsDeflection;
  for (int i = 0; i < 4; i++) {
    int sign = (deflection[i] >= 0) ? 1 : -1;
    Properties::spanFactor[i] = sign * initScale;
  }
  snprintf(buf, sizeof(buf), "Initial scale: %.8f g/count", initScale);
  Logger::log(buf);

  // ── Step 3: corner trim ───────────────────────────────────────────────────
  Logger::log("[3/3] Corner trim: place weight over each corner one at a time.");

  for (int k = 1; k <= 4; k++) {
    snprintf(buf, sizeof(buf),
             "  Corner %d/4: place %.0fg over corner %d, then send any key.",
             k, CORNER_WEIGHT_G, k);
    waitForSerial(buf);

    float delta[4];
    int domCh = 0;
    float maxAbsDelta = 0;
    for (int ch = 1; ch <= 4; ch++) {
      float val = hw->readAverage(ch, CAL_SAMPLES);
      delta[ch - 1] = val - Properties::zeroOffset[ch - 1];
      if (fabsf(delta[ch - 1]) > maxAbsDelta) {
        maxAbsDelta = fabsf(delta[ch - 1]);
        domCh = ch - 1;
      }
    }

    float measured = 0;
    for (int i = 0; i < 4; i++) measured += Properties::spanFactor[i] * delta[i];

    snprintf(buf, sizeof(buf), "  Reading: %.1f g  (dominant CH%d)", measured, domCh + 1);
    Logger::log(buf);

    if (maxAbsDelta > 1000) {
      float sumOther = measured - Properties::spanFactor[domCh] * delta[domCh];
      Properties::spanFactor[domCh] = (CORNER_WEIGHT_G - sumOther) / delta[domCh];
      snprintf(buf, sizeof(buf), "  CH%d span adjusted to %.8f g/count",
               domCh + 1, Properties::spanFactor[domCh]);
      Logger::log(buf);
    } else {
      snprintf(buf, sizeof(buf), "  CH%d: deflection too small, skipped.", domCh + 1);
      Logger::log(buf);
    }
  }

  // ── Save & summary ────────────────────────────────────────────────────────
  Properties::calibrationValid = true;
  Properties::saveConfigToNVS();

  Logger::log("=== Calibration values ===");
  Logger::log("  Ch  zeroOffset       spanFactor");
  for (int i = 0; i < 4; i++) {
    snprintf(buf, sizeof(buf), "  %d   %12.0f    %.8f g/count",
             i + 1, Properties::zeroOffset[i], Properties::spanFactor[i]);
    Logger::log(buf);
  }
  Logger::log("=== Calibration complete, saved to NVS ===");
}
