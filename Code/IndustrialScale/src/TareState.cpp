#include "TareState.h"
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Arduino.h>
#include "Properties.h"

#define DISPLAY_X_MARGIN      10
#define DISPLAY_Y_TITLE       20
#define DISPLAY_Y_INSTRUCTION 50
#define DISPLAY_Y_PROGRESS    65
#define DISPLAY_Y_COMPLETE    110
#define PROGRESS_BAR_WIDTH    240
#define PROGRESS_BAR_HEIGHT   20
#define PROGRESS_BAR_PADDING  2

#define TARE_CHANNEL_COUNT    4
#define TARE_SAMPLES          10   // averaged samples per channel

TareState::TareState() {}

void TareState::enter() {
  Logger::log("Enter Tare State");

  hw = HwContext::get();

  updateDisplayInitText();
  updateDisplayAddWeight();
  drawProgressBarOutline();

  performTare();

  updateDisplayTaringComplete();

  Logger::log("Tare screen complete");
}

void TareState::update() {}

void TareState::exit() {
  Logger::log("Exit Tare State");
}

StateType TareState::nextState() {
  return StateType::BLE_INIT;
}

void TareState::updateDisplayInitText() {
  hw->display->fillScreen(GxEPD_WHITE);
  hw->display->setCursor(DISPLAY_X_MARGIN, DISPLAY_Y_TITLE);
  hw->display->setFont(&FreeSansBold12pt7b);
  hw->display->print("Taring");
  hw->display->display(true);
}

void TareState::updateDisplayAddWeight() {
  hw->display->setCursor(DISPLAY_X_MARGIN, DISPLAY_Y_INSTRUCTION);
  hw->display->setFont(&FreeSans9pt7b);
  hw->display->print("Remove all weight from scale");
  hw->display->display(true);
}

void TareState::drawProgressBarOutline() {
  hw->display->drawRect(DISPLAY_X_MARGIN, DISPLAY_Y_PROGRESS,
                        PROGRESS_BAR_WIDTH, PROGRESS_BAR_HEIGHT, GxEPD_BLACK);
  hw->display->display(true);
}

void TareState::performTare() {
  // Taring = re-capture the raw zero reading of each channel so the empty scale
  // reads 0 g. spanFactor (the calibrated corner balancing) is left untouched,
  // so the corner compensation from calibration is preserved.
  Logger::log("Taring: capturing zero offset per channel...");

  if (!Properties::calibrationValid) {
    Logger::log("Note: scale not calibrated — capturing zero offset anyway.");
  }

  char buf[48];
  for (int ch = 1; ch <= TARE_CHANNEL_COUNT; ch++) {
    float zero = hw->readAverage(ch, TARE_SAMPLES);
    Properties::zeroOffset[ch - 1] = zero;

    snprintf(buf, sizeof(buf), "  CH%d zero: %ld", ch, (long)zero);
    Logger::log(buf);

    int fillWidth = (ch * (PROGRESS_BAR_WIDTH / TARE_CHANNEL_COUNT)) - PROGRESS_BAR_PADDING;
    if (fillWidth > 0) {
      hw->display->fillRect(DISPLAY_X_MARGIN + 1, DISPLAY_Y_PROGRESS + 1,
                            fillWidth, PROGRESS_BAR_HEIGHT - PROGRESS_BAR_PADDING, GxEPD_BLACK);
    }
    hw->display->display(true);
  }

  Properties::saveConfigToNVS();
  Logger::log("Taring complete — zero offsets saved to NVS.");
}

void TareState::updateDisplayTaringComplete() {
  hw->display->setCursor(DISPLAY_X_MARGIN, DISPLAY_Y_COMPLETE);
  hw->display->setFont(&FreeSans9pt7b);
  hw->display->print("Taring complete!");
  hw->display->display(true);
}
