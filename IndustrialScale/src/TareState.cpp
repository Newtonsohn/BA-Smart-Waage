#include "TareState.h"
#include "Properties.h"

// TareState is deprecated — calibration is handled by CalibrationState (4-channel).
// Kept as a stub so the state machine switch compiles.

TareState::TareState() {}

void TareState::enter() {
  Logger::log("TareState: deprecated, transitioning to BLE_INIT.");
}

void TareState::update() {}

void TareState::exit() {
  Logger::log("Exit Tare State");
}

StateType TareState::nextState() {
  return StateType::BLE_INIT;
}

void TareState::updateDisplayInitText() {}
void TareState::updateDisplayAddWeight() {}
void TareState::updateDisplayProgressBar() {}
void TareState::updateDisplayTaringComplete() {}
