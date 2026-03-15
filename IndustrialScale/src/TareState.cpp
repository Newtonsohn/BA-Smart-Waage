#include "TareState.h"
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include "Properties.h"

#define DISPLAY_X_MARGIN 10
#define DISPLAY_Y_TITLE 20
#define DISPLAY_Y_ADD_WEIGHT 50
#define DISPLAY_Y_TARE_COMPLETE 110
#define DISPLAY_Y_PROGRESS_BAR 65
#define PROGRESS_BAR_WIDTH 240
#define PROGRESS_BAR_HEIGHT 20
#define PROGRESS_BAR_PADDING 2

#define PROGRESS_BAR_DELAY 250

TareState::TareState() {}

void TareState::enter() {
  Logger::log("Enter Tare State");

  hw = HwContext::get();

  updateDisplayInitText();
  updateDisplayAddWeight();

  // Calculate zeroOffset
  Properties::zeroOffset = hw->averageScaleReading();

  updateDisplayProgressBar();

  // Calculate calFactor
  Properties::calFactor = hw->averageScaleReading() - Properties::zeroOffset;

  updateDisplayTaringComplete();

  Logger::log("Taring complete");
}

void TareState::update() {}

void TareState::exit() {
  Logger::log("Exit Tare State");
}

StateType TareState::nextState() {
  return StateType::BLE_INIT;
}

void TareState::updateDisplayInitText() {
  // Title
  hw->display->setCursor(DISPLAY_X_MARGIN, 20);
  hw->display->setFont(&FreeSansBold12pt7b);
  hw->display->print("Taring");
  hw->display->display(true);
}

void TareState::updateDisplayAddWeight() {
  // Add weight Text
  hw->display->setCursor(10, 50);
  hw->display->setFont(&FreeSans9pt7b);
  hw->display->print("Please add a weight of 1 kg");
  hw->display->display(true);
}
void TareState::updateDisplayTaringComplete() {
  // Thank you text
  hw->display->setCursor(DISPLAY_X_MARGIN, 110);
  hw->display->setFont(&FreeSans9pt7b);
  hw->display->print("Taring completed, thank you! :)");
  hw->display->display(true);
}
void TareState::updateDisplayProgressBar() {
  // Progress bar
  for (int i = 0; i <= Properties::tareTime; i++) {
    // Border
    hw->display->drawRect(DISPLAY_X_MARGIN, DISPLAY_Y_PROGRESS_BAR, PROGRESS_BAR_WIDTH, PROGRESS_BAR_HEIGHT, GxEPD_BLACK);

    // Filled
    int fillWidth = (i * (PROGRESS_BAR_WIDTH / Properties::tareTime)) - PROGRESS_BAR_PADDING;
    hw->display->fillRect(DISPLAY_X_MARGIN + 1, DISPLAY_Y_PROGRESS_BAR + 1, fillWidth, PROGRESS_BAR_HEIGHT - PROGRESS_BAR_PADDING, GxEPD_BLACK);
    hw->display->display(true);

    // Delay of 250 ms since display update takes approx. 750 ms which together sums up to approx. 1s
    delay(PROGRESS_BAR_DELAY);
  }
}
