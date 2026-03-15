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
    unsigned long ms = millis();
    unsigned long seconds = ms / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;
    char timestamp[16];
    snprintf(timestamp, sizeof(timestamp), "[%02lu:%02lu:%02lu] ", hours, minutes % 60, seconds % 60);
    Serial.print(timestamp);
    Serial.println(message);
  }
}
