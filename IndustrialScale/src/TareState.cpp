#include "TareState.h"
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include "Properties.h"

#define DISPLAY_X_MARGIN      10
#define DISPLAY_Y_TITLE       20
#define DISPLAY_Y_INSTRUCTION 50
#define DISPLAY_Y_PROGRESS    65
#define DISPLAY_Y_COMPLETE    110
#define PROGRESS_BAR_WIDTH    240
#define PROGRESS_BAR_HEIGHT   20
#define PROGRESS_BAR_PADDING  2
#define PROGRESS_STEP_DELAY   250

TareState::TareState() {}

void TareState::enter() {
  Logger::log("Enter Tare State");

  hw = HwContext::get();

  updateDisplayInitText();
  updateDisplayAddWeight();
  updateDisplayProgressBar();
  updateDisplayTaringComplete();

  Logger::log("Tare screen complete");
}

void TareState::update() {}

void TareState::exit() {
  Logger::log("Exit Tare State");
}

StateType TareState::nextState() {
  return StateType::CALIBRATION_CHECK;
}

void TareState::updateDisplayInitText() {
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

void TareState::updateDisplayProgressBar() {
  int steps = Properties::tareTime;
  for (int i = 0; i <= steps; i++) {
    hw->display->drawRect(DISPLAY_X_MARGIN, DISPLAY_Y_PROGRESS,
                          PROGRESS_BAR_WIDTH, PROGRESS_BAR_HEIGHT, GxEPD_BLACK);
    int fillWidth = (i * (PROGRESS_BAR_WIDTH / steps)) - PROGRESS_BAR_PADDING;
    if (fillWidth > 0) {
      hw->display->fillRect(DISPLAY_X_MARGIN + 1, DISPLAY_Y_PROGRESS + 1,
                            fillWidth, PROGRESS_BAR_HEIGHT - PROGRESS_BAR_PADDING, GxEPD_BLACK);
    }
    hw->display->display(true);
    delay(PROGRESS_STEP_DELAY);
  }
}

void TareState::updateDisplayTaringComplete() {
  hw->display->setCursor(DISPLAY_X_MARGIN, DISPLAY_Y_COMPLETE);
  hw->display->setFont(&FreeSans9pt7b);
  hw->display->print("Taring complete!");
  hw->display->display(true);
}
