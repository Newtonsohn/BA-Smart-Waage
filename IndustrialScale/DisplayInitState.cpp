#include "DisplayInitState.h"
#include "Properties.h"
#include "StateType.h"

#define DISPLAY_ROTATION_90_DEGREE 1

DisplayInitState::DisplayInitState() {}

void DisplayInitState::enter() {
  Logger::log("Load DisplayInit State");

  hw = HwContext::get();

  // Setup of the display
  hw->display = std::make_shared<GxEPD2_BW<GxEPD2_290_BS, GxEPD2_290_BS::HEIGHT>>(
    GxEPD2_290_BS(Properties::DISPLAY_CS_PIN, Properties::DISPLAY_DC_PIN,
                  Properties::DISPLAY_RESET_PIN, Properties::DISPLAY_BUSY_PIN));

  hw->display->init();
  hw->display->setRotation(DISPLAY_ROTATION_90_DEGREE);
  hw->display->fillScreen(GxEPD_WHITE);
  hw->display->setTextColor(GxEPD_BLACK);
}
void DisplayInitState::update() {}

void DisplayInitState::exit() {
  Logger::log("Exit Display initialization State");
}

StateType DisplayInitState::nextState() {
  if (Properties::sendDataFailed) {
    return StateType::FAILURE;
  } else {
    return StateType::ADC_INIT;
  }
}
