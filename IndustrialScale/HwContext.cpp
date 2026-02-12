#include "HwContext.h"
#include "Properties.h"

std::shared_ptr<HwContext> HwContext::get() {
  static std::shared_ptr<HwContext> instance = std::make_shared<HwContext>();
  return instance;
}

long HwContext::averageScaleReading() {
  long sum = 0;

  for (int i = 0; i < Properties::SCALE_READING_SAMPLE_COUNT; i++) {
    sum += scale->read();
    delay(Properties::SCALE_READING_SAMPLE_DELAY);
  }

  return sum / Properties::SCALE_READING_SAMPLE_COUNT;
}