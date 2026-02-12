#include "Logger.h"
#include "Properties.h"

#define BAUDRATE 115200

void Logger::enableLogging(bool value) {
  Properties::loggingEnabled = value;

  Serial.begin(BAUDRATE);
  Serial.println("Debugging mode is switched on");
}

void Logger::log(const char* message) {
  if (Properties::loggingEnabled) {
    Serial.println(message);
  }
}
