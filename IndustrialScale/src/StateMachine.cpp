#include <cmath>

#include "StateType.h"
#include "StateMachine.h"

// Include states
#include "DisplayInitState.h"
#include "AdcInitState.h"
#include "TareState.h"
#include "MeasureState.h"
#include "BleInitState.h"
#include "ConfigurationState.h"
#include "SendDataState.h"
#include "DeepSleepState.h"
#include "FailureState.h"

#include "Properties.h"

StateMachine::StateMachine() {
  if (Properties::wakeUpCauseIsTimer) {
    // Load values from NVS after wakeup
    Properties::loadConfigFromNVS();

    // Increase the hearbeat counter by 1
    Properties::heartbeatCounter++;

    // Since the hardware awakes from deep sleep we start with ADC initialization
    currentStateType = StateType::ADC_INIT;
    currentState = new AdcInitState();
  } else {
    // Since the hardware was powered on we start with display initialization
    currentStateType = StateType::DISPLAY_INIT;
    currentState = new DisplayInitState();
  }

  Logger::log("Starting State Machine");

  // Call enter within current state
  currentState->enter();
}

void StateMachine::updateState() {
  if (!currentState) {
    return;
  }

  // Call update within current state
  currentState->update();

  StateType nextStateType = currentState->nextState();
  if (nextStateType != currentStateType)
    transitionTo(nextStateType);
}

void StateMachine::transitionTo(StateType newState) {
  if (currentState) {
    // Call exit within current state
    currentState->exit();
    delete currentState;
  }

  currentStateType = newState;

  switch (newState) {
    case StateType::DISPLAY_INIT: currentState = new DisplayInitState(); break;
    case StateType::ADC_INIT: currentState = new AdcInitState(); break;
    case StateType::TARE: currentState = new TareState(); break;
    case StateType::MEASURE: currentState = new MeasureState(); break;
    case StateType::BLE_INIT: currentState = new BleInitState(); break;
    case StateType::CONFIGURATION: currentState = new ConfigurationState(); break;
    case StateType::SEND_DATA: currentState = new SendDataState(); break;
    case StateType::DEEP_SLEEP: currentState = new DeepSleepState(); break;
    case StateType::FAILURE: currentState = new FailureState(); break;
  }

  if (currentState) {
    // Call enter within current state
    currentState->enter();
  }
}
