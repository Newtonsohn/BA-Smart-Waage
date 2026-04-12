#include <Arduino.h>
#include <HX711.h>

// Same pins as IndustrialScale
#define ADC_DT  26
#define ADC_SCK 22

// Same sampling as IndustrialScale (Properties.cpp)
#define SAMPLE_COUNT 20
#define SAMPLE_DELAY_MS 5

// Calibration reference weight in grams (used when pressing 'c')
#define CAL_WEIGHT_GRAMS 1000.0f

float zeroOffset = 0.0f;
float calFactor   = 1.0f;  // counts per kg — set by calibration

HX711 scale;

long averageReading() {
  long sum = 0;
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    sum += scale.read();
    delay(SAMPLE_DELAY_MS);
  }
  return sum / SAMPLE_COUNT;
}

void waitForStableReading() {
  const long STABILITY_THRESHOLD = 2000;
  const int  STABLE_REQUIRED     = 3;

  Serial.print("Waiting for stable readings");
  long prev = averageReading();
  int stableCount = 0;

  while (stableCount < STABLE_REQUIRED) {
    long current = averageReading();
    if (abs(current - prev) < STABILITY_THRESHOLD) {
      stableCount++;
    } else {
      stableCount = 0;
    }
    prev = current;
    Serial.print(".");
  }
  Serial.println(" stable.");
}

void doTare() {
  Serial.println("Taring — keep scale empty...");
  waitForStableReading();
  zeroOffset = (float)averageReading();
  Serial.print("Tare done. Zero offset: ");
  Serial.println(zeroOffset, 0);
}

// Call with known weight on scale to set calFactor
void doCalibrate() {
  Serial.print("Calibrating with ");
  Serial.print(CAL_WEIGHT_GRAMS, 0);
  Serial.println(" g — keep weight on scale...");
  waitForStableReading();
  long raw   = averageReading();
  long delta = raw - (long)zeroOffset;
  // calFactor = counts per kg  →  grams = (delta / calFactor) * 1000
  calFactor = (float)delta / (CAL_WEIGHT_GRAMS / 1000.0f);
  Serial.print("Calibration done. calFactor = ");
  Serial.print(calFactor, 2);
  Serial.println("  (paste this into Properties.cpp as calFactor)");
}

void printHeader() {
  Serial.println("RAW\t\tDELTA\t\tGRAMS");
  Serial.println("-------------------------------------------");
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("=== HX711 Debug ===");
  Serial.println("Commands: 't' = tare   'c' = calibrate with 1 kg");
  Serial.println();

  scale.begin(ADC_DT, ADC_SCK);

  Serial.print("Waiting for ADC");
  while (!scale.is_ready()) {
    Serial.print(".");
    delay(100);
  }
  Serial.println(" ready.");

  doTare();
  Serial.println();
  printHeader();
}

void loop() {
  if (!scale.is_ready()) {
    Serial.println("ADC not ready");
    delay(200);
    return;
  }

  if (Serial.available()) {
    char cmd = Serial.read();
    if (cmd == 't') {
      Serial.println();
      doTare();
      printHeader();
    } else if (cmd == 'c') {
      Serial.println();
      doCalibrate();
      printHeader();
    }
  }

  long raw   = averageReading();
  long delta = raw - (long)zeroOffset;

  Serial.print(raw);
  Serial.print("\t\t");
  Serial.print(delta);
  Serial.print("\t\t");

  if (calFactor == 1.0f) {
    Serial.println("--- put 1kg on scale and send 'c' ---");
  } else {
    float grams = ((float)delta / calFactor) * 1000.0f;
    Serial.print(grams, 1);
    Serial.println(" g");
  }

  delay(500);
}
