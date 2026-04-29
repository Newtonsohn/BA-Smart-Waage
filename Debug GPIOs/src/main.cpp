#include <Arduino.h>

#define PIN_A 4
#define PIN_B 32

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(PIN_A, OUTPUT);
  pinMode(PIN_B, OUTPUT);

  Serial.println("GPIO test: IO4 and IO32 — toggling HIGH/LOW every 5s");
}

void loop() {
  digitalWrite(PIN_A, HIGH);
  digitalWrite(PIN_B, HIGH);
  Serial.println("IO4 = HIGH, IO32 = HIGH");
  delay(5000);

  digitalWrite(PIN_A, LOW);
  digitalWrite(PIN_B, LOW);
  Serial.println("IO4 = LOW,  IO32 = LOW");
  delay(5000);
}
