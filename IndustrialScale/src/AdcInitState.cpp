#include "AdcInitState.h"
#include "Properties.h"
#include "StateType.h"

#define ADC_CHECK_DELAY_MS 100
#define ADC_STABILIZATION_DELAY_COLD 500   // power-cycle: HX711 needs time to settle
#define ADC_STABILIZATION_DELAY_WARM 100   // timer wake-up: HX711 was already running

AdcInitState::AdcInitState() {}

void AdcInitState::enter() {
  Logger::log("Enter ADC initialization State");

  hw = HwContext::get();

  hw->scale = std::make_shared<HX711>();
  hw->scale->begin(Properties::ADC_DT, Properties::ADC_SCK);
}

void AdcInitState::update() {
  hw = HwContext::get();

  if (hw->scale->is_ready()) {
    Logger::log("ADC ready");
    adcIsReady = true;
  } else {
    Logger::log("Waiting for ADC...");
    delay(ADC_CHECK_DELAY_MS);
  }
}

void AdcInitState::exit() {
  Logger::log("Exit ADC initialization State");
  delay(Properties::wakeUpCauseIsTimer ? ADC_STABILIZATION_DELAY_WARM : ADC_STABILIZATION_DELAY_COLD);
}

StateType AdcInitState::nextState() {
  // Keep current state until ADC is ready
  if (!adcIsReady) {
    return StateType::ADC_INIT;
  }

  if (Properties::wakeUpCauseIsTimer) {
    return StateType::MEASURE;
  } else {
    return StateType::TARE;
  }
}
