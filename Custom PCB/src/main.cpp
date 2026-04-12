#include <Arduino.h>

// --- Pin definitions ---
#define SCLK_PIN     15   // D15  → ADS1234 SCLK
#define DRDY_PIN     2    // D2   → ADS1234 DOUT/!DRDY
#define PDWN_PIN     26   // D26  → ADS1234 !PDWN  (active LOW)
#define DMS_PWR_PIN  17   // IO17 → P-MOSFET gate (high-side), active LOW (!EN_DMS)

// A0 = A1 = 3.3V     → Channel 4 (AINP4/AINN4) hardwired on PCB
// GAIN0 = GAIN1 = GND → Gain = 1  hardwired on PCB
// CLKIN = GND         → internal oscillator (~10 SPS)

// --- Sampling ---
#define SAMPLE_COUNT        50
#define SAMPLE_DELAY_MS     50
#define STABILITY_THRESHOLD 50000  // raw counts
#define STABLE_REQUIRED     3

// --- Calibration ---
#define CAL_WEIGHT_GRAMS 2000.0f

float zeroOffset = 0.0f;
float calFactor  = 1.0f;   // raw counts per kg

// ============================================================
//  ADS1234 low-level driver
// ============================================================

void adsBegin() {
  pinMode(SCLK_PIN,    OUTPUT);
  pinMode(DRDY_PIN,    INPUT_PULLUP);
  pinMode(PDWN_PIN,    OUTPUT);
  pinMode(DMS_PWR_PIN, OUTPUT);

  digitalWrite(SCLK_PIN,    LOW);
  digitalWrite(PDWN_PIN,    LOW);    // hold in power-down first
  digitalWrite(DMS_PWR_PIN, LOW);    // !EN_DMS LOW → P-MOSFET ON → 3.3 V to load cells
  delay(100);                        // let supply stabilise
  digitalWrite(PDWN_PIN,    HIGH);   // release power-down → ADC starts converting
  delay(500);                        // wait for first conversion (~10 SPS = 100 ms/sample)
}

bool adsIsReady() {
  return digitalRead(DRDY_PIN) == LOW;
}

long adsReadRaw() {
  long result = 0;
  for (int i = 0; i < 24; i++) {
    digitalWrite(SCLK_PIN, HIGH);
    delayMicroseconds(1);
    result = (result << 1) | digitalRead(DRDY_PIN);
    digitalWrite(SCLK_PIN, LOW);
    delayMicroseconds(1);
  }
  if (result & 0x800000) {
    result |= 0xFF000000;
  }
  return result;
}

long adsRead() {
  unsigned long deadline = millis() + 2000;
  while (!adsIsReady()) {
    if (millis() > deadline) {
      Serial.println("ADS1234 timeout!");
      return 0;
    }
  }
  return adsReadRaw();
}

// ============================================================
//  Averaging & stability
// ============================================================

long averageReading() {
  long sum = 0;
  for (int i = 0; i < SAMPLE_COUNT; i++) {
    sum += adsRead();
    delay(SAMPLE_DELAY_MS);
  }
  return sum / SAMPLE_COUNT;
}

void waitForStableReading() {
  Serial.println("RAW\t\tGRAMS\t\tSTABLE");
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

    long delta = current - (long)zeroOffset;
    Serial.print(current);
    Serial.print("\t\t");
    if (calFactor == 1.0f) {
      Serial.print("---");
    } else {
      float grams = ((float)delta / calFactor) * 1000.0f;
      Serial.print(grams, 1);
      Serial.print(" g");
    }
    Serial.print("\t\t");
    Serial.print(stableCount);
    Serial.print("/");
    Serial.println(STABLE_REQUIRED);
  }
  Serial.println("Stable!");
}

// ============================================================
//  Guided startup calibration sequence
// ============================================================

void runCalibrationSequence() {
  // Step 1: Tare with empty scale
  Serial.println("=== Step 1: Tare ===");
  Serial.println("Remove all weight from scale, then press Enter...");
  while (!Serial.available()) delay(100);
  while (Serial.available()) Serial.read();  // flush

  Serial.println("Taring...");
  waitForStableReading();
  zeroOffset = (float)averageReading();
  Serial.print("Tare done. zeroOffset = ");
  Serial.println(zeroOffset, 0);
  Serial.println();

  // Step 2: Calibrate with 2 kg
  Serial.print("=== Step 2: Calibration with ");
  Serial.print(CAL_WEIGHT_GRAMS, 0);
  Serial.println(" g ===");
  Serial.print("Place exactly ");
  Serial.print(CAL_WEIGHT_GRAMS, 0);
  Serial.println(" g on the scale, then press Enter...");
  while (!Serial.available()) delay(100);
  while (Serial.available()) Serial.read();  // flush

  Serial.println("Calibrating...");
  waitForStableReading();
  long raw  = averageReading();
  long delta = raw - (long)zeroOffset;
  calFactor  = (float)delta / (CAL_WEIGHT_GRAMS / 1000.0f);
  Serial.print("Calibration done. calFactor = ");
  Serial.print(calFactor, 2);
  Serial.println();
  Serial.println();

  // Step 3: Ready
  Serial.println("=== Measuring ===");
  Serial.println("GRAMS");
  Serial.println("-----");
}

// ============================================================
//  Arduino entry points
// ============================================================

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("=== ADS1234 Scale ===");
  Serial.println();

  adsBegin();

  Serial.print("Waiting for ADC");
  unsigned long t = millis();
  while (!adsIsReady()) {
    if (millis() - t > 5000) {
      Serial.println(" TIMEOUT — check wiring (SCLK=D15, DRDY=D2, !PDWN=D26, !EN_DMS=IO17).");
      while (true) delay(1000);
    }
    Serial.print(".");
    delay(100);
  }
  Serial.println(" ready.");
  Serial.println();

  runCalibrationSequence();
}

void loop() {
  long raw   = averageReading();
  long delta = raw - (long)zeroOffset;
  float grams = ((float)delta / calFactor) * 1000.0f;

  Serial.print(grams, 1);
  Serial.println(" g");
}
