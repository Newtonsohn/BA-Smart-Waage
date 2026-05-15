#pragma once
#include <Arduino.h>

#ifdef ENABLE_DEBUG_TOGGLE

#define DEBUG_TOGGLE_PIN 17

inline void debugToggleSetup() {
  pinMode(DEBUG_TOGGLE_PIN, OUTPUT);
  digitalWrite(DEBUG_TOGGLE_PIN, LOW);
}

inline void debugToggle() {
  digitalWrite(DEBUG_TOGGLE_PIN, !digitalRead(DEBUG_TOGGLE_PIN));
}

#else

inline void debugToggleSetup() {}
inline void debugToggle() {}

#endif
