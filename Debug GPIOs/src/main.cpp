#include <Arduino.h>

#define PIN_DMS_PWR 17  // P-MOSFET gate, HIGH = off, LOW = on (active LOW)

void printHelp() {
  Serial.println("=== DMS Power Test ===");
  Serial.println("Commands:");
  Serial.println("  1 -> DMS power ON  (GPIO17 LOW  -> MOSFET conducts)");
  Serial.println("  0 -> DMS power OFF (GPIO17 HIGH -> MOSFET off)");
  Serial.println("  s -> status");
  Serial.println("  h -> help");
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(PIN_DMS_PWR, OUTPUT);
  digitalWrite(PIN_DMS_PWR, HIGH);  // start with power OFF (safe default)

  printHelp();
  Serial.println("DMS power: OFF");
}

void loop() {
  if (!Serial.available()) return;

  char cmd = Serial.read();

  switch (cmd) {
    case '1':
      digitalWrite(PIN_DMS_PWR, LOW);
      Serial.println("DMS power: ON  (GPIO17 = LOW)");
      break;
    case '0':
      digitalWrite(PIN_DMS_PWR, HIGH);
      Serial.println("DMS power: OFF (GPIO17 = HIGH)");
      break;
    case 's':
      Serial.printf("DMS power: %s (GPIO17 = %s)\n",
        digitalRead(PIN_DMS_PWR) == LOW ? "ON" : "OFF",
        digitalRead(PIN_DMS_PWR) == LOW ? "LOW" : "HIGH");
      break;
    case 'h':
      printHelp();
      break;
    default:
      break;
  }
}
