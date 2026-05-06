#include <Arduino.h>

#define PIN_DRDY_DOUT  2
#define PIN_SCLK       15
<<<<<<< Updated upstream
#define PIN_PDWN       26
#define PIN_DMS_PWR    17
#define PIN_A0         32
#define PIN_A1         4

#define CAL_WEIGHT_G   3000.0f  // known calibration weight in grams
#define CAL_SAMPLES    5        // samples per channel (~2s per channel at 10SPS)

static float zeroOffset[4];
static float scaleFactor;       // grams per (sum of sign-corrected counts)
static int   channelSign[4];    // +1 or -1 polarity correction per channel
static bool calibrated = false;

// ── ADS1234 low-level ─────────────────────────────────────────────────────────

void setChannel(int ch) {
    if (ch < 1 || ch > 4) return;
    int i = ch - 1;
    digitalWrite(PIN_A0, i & 0x01);
    digitalWrite(PIN_A1, (i >> 1) & 0x01);
=======
#define PIN_PDWN       26  // ADS1234 !PDWN, active low
#define PIN_DMS_PWR    17  // MOSFET DMS power, active low
#define PIN_A0         32  // ADS1234 channel select bit 0
#define PIN_A1         4   // ADS1234 channel select bit 1

static int currentChannel = 1;

// Channel 1-4: A1A0 = 00, 01, 10, 11
void setChannel(int ch) {
    if (ch < 1 || ch > 4) return;
    currentChannel = ch;
    int idx = ch - 1;
    digitalWrite(PIN_A0, idx & 0x01);
    digitalWrite(PIN_A1, (idx >> 1) & 0x01);
>>>>>>> Stashed changes
}

int32_t readADS1234() {
    unsigned long t = millis();
    unsigned long lastPrint = t;
    while (digitalRead(PIN_DRDY_DOUT) == HIGH) {
        if (millis() - lastPrint > 3000) {
            Serial.printf("Waiting for DRDY... (%lus elapsed)\n", (millis() - t) / 1000);
            lastPrint = millis();
        }
    }
    uint32_t raw = 0;
    for (int i = 0; i < 24; i++) {
        digitalWrite(PIN_SCLK, HIGH);
        delayMicroseconds(2);
        raw = (raw << 1) | digitalRead(PIN_DRDY_DOUT);
        digitalWrite(PIN_SCLK, LOW);
        delayMicroseconds(2);
    }
    // 25th clock: force DRDY high (Figure 7-10)
    digitalWrite(PIN_SCLK, HIGH); delayMicroseconds(2);
    digitalWrite(PIN_SCLK, LOW);  delayMicroseconds(2);

    if (raw & 0x800000) raw |= 0xFF000000;
    return (int32_t)raw;
}

// Switch to channel and apply PDWN workaround for CH1 when DRDY doesn't reset.
// CH4(11)→CH1(00) has no rising edge on A0/A1 so ADS1234 never raises DRDY.
int32_t readChannel(int ch) {
    setChannel(ch);
    if (ch == 1 && digitalRead(PIN_DRDY_DOUT) == LOW) {
        digitalWrite(PIN_PDWN, LOW);
        delayMicroseconds(500);
        digitalWrite(PIN_PDWN, HIGH);
    }
    return readADS1234();
}

// Read n averaged samples; first sample includes channel switch + DRDY settle.
float readAverage(int ch, int n) {
    long long sum = readChannel(ch);
    for (int i = 1; i < n; i++) {
        sum += readADS1234();
    }
    return (float)sum / n;
}

// ── Serial helpers ────────────────────────────────────────────────────────────

void waitForSerial(const char* msg) {
    Serial.println(msg);
    while (Serial.available()) Serial.read();  // flush pending bytes
    while (!Serial.available()) delay(50);
    while (Serial.available()) Serial.read();  // consume input
}

// ── Calibration ───────────────────────────────────────────────────────────────

void runCalibration() {
    int estimatedSeconds = (int)(CAL_SAMPLES * 4 * 0.4f);

    waitForSerial("\n[Step 1/2] Remove ALL weight from scale.\nPress any key when ready...");
    Serial.printf("Measuring zero offset (~%ds)...\n", estimatedSeconds);
    for (int ch = 1; ch <= 4; ch++) {
        zeroOffset[ch - 1] = readAverage(ch, CAL_SAMPLES);
        Serial.printf("  CH%d: %.0f\n", ch, zeroOffset[ch - 1]);
    }

    waitForSerial("\n[Step 2/2] Place calibration weight (%.0fg) on scale.\nPress any key when ready...");
    Serial.printf("Measuring %.0fg (~%ds)...\n", CAL_WEIGHT_G, estimatedSeconds);

    float deflection[4];
    float totalAbsDeflection = 0;
    for (int ch = 1; ch <= 4; ch++) {
        float val = readAverage(ch, CAL_SAMPLES);
        deflection[ch - 1] = val - zeroOffset[ch - 1];
        channelSign[ch - 1] = (deflection[ch - 1] >= 0) ? 1 : -1;
        totalAbsDeflection += fabsf(deflection[ch - 1]);
        Serial.printf("  CH%d: %+.0f counts  [%s]\n",
                      ch, deflection[ch - 1],
                      channelSign[ch - 1] > 0 ? "normal" : "inverted");
    }

    if (totalAbsDeflection < 10000) {
        Serial.println("ERROR: Deflection too small — check load cell wiring.");
        Serial.println("Falling back to raw ADC mode.");
        return;
    }

    scaleFactor = CAL_WEIGHT_G / totalAbsDeflection;
    calibrated = true;
    Serial.printf("\nCalibration done!  Scale factor: %.6f g/count\n\n", scaleFactor);
}

// ── Setup ─────────────────────────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);

<<<<<<< Updated upstream
    pinMode(PIN_SCLK, OUTPUT);    digitalWrite(PIN_SCLK, LOW);
    pinMode(PIN_DMS_PWR, OUTPUT); digitalWrite(PIN_DMS_PWR, HIGH);
    pinMode(PIN_PDWN, OUTPUT);    digitalWrite(PIN_PDWN, HIGH);
=======
    pinMode(PIN_SCLK, OUTPUT);
    digitalWrite(PIN_SCLK, LOW);

    pinMode(PIN_A0, OUTPUT);
    pinMode(PIN_A1, OUTPUT);

    pinMode(PIN_DMS_PWR, OUTPUT);
    digitalWrite(PIN_DMS_PWR, HIGH); // MOSFET off initially

    pinMode(PIN_PDWN, OUTPUT);
    digitalWrite(PIN_PDWN, HIGH);    // default HIGH (running) until toggle below

>>>>>>> Stashed changes
    pinMode(PIN_DRDY_DOUT, INPUT);
    pinMode(PIN_A0, OUTPUT);
    pinMode(PIN_A1, OUTPUT);
    setChannel(1);

<<<<<<< Updated upstream
    Serial.println("\n=== Smart Scale ===");
    Serial.println("  r  =  raw ADC output");
    Serial.println("  c  =  calibrate then show weight");
    Serial.print("Choice: ");

    char mode = 0;
    while (mode != 'r' && mode != 'c') {
        if (Serial.available()) mode = Serial.read();
        delay(50);
    }
    Serial.println(mode);

    // Power-on sequence (after mode selected so DMS is fresh)
    digitalWrite(PIN_DMS_PWR, LOW);
=======
    setChannel(1); // start on channel 1

    // Power-on sequence
    digitalWrite(PIN_DMS_PWR, LOW);  // enable DMS power (active low)
>>>>>>> Stashed changes
    delay(100);
    digitalWrite(PIN_PDWN, LOW);
    delay(10);
    digitalWrite(PIN_PDWN, HIGH);
    delay(500);

<<<<<<< Updated upstream
    Serial.printf("ADC ready. DRDY: %s\n",
                  digitalRead(PIN_DRDY_DOUT) ? "HIGH (not ready)" : "LOW (ready)");

    if (mode == 'c') {
        runCalibration();
    }
=======
    Serial.printf("Init done. DRDY is currently: %s\n",
                  digitalRead(PIN_DRDY_DOUT) ? "HIGH (not ready)" : "LOW (ready!)");
    Serial.println("Send 1-4 to switch channel.");
>>>>>>> Stashed changes
}

// ── Loop ──────────────────────────────────────────────────────────────────────

void loop() {
    if (Serial.available()) {
        char c = Serial.read();
<<<<<<< Updated upstream
        if (c == 'f') {
            digitalWrite(PIN_PDWN, LOW);
            digitalWrite(PIN_DMS_PWR, HIGH);
            Serial.println("Flash mode: IO2 released. Flash now, then reset.");
            while (true) delay(1000);
        }
    }

    int32_t values[4];
    for (int ch = 1; ch <= 4; ch++) {
        values[ch - 1] = readChannel(ch);
    }

    if (calibrated) {
        float totalCounts = 0;
        for (int ch = 0; ch < 4; ch++) {
            totalCounts += channelSign[ch] * (values[ch] - zeroOffset[ch]);
        }
        Serial.printf("%.1f g\n", totalCounts * scaleFactor);
    } else {
        Serial.printf("CH1:%d CH2:%d CH3:%d CH4:%d\n",
                      values[0], values[1], values[2], values[3]);
    }

=======
        if (c >= '1' && c <= '4') {
            setChannel(c - '0');
            Serial.printf(">> Switched to channel %d (A1=%d A0=%d)\n",
                          currentChannel,
                          (currentChannel - 1) >> 1,
                          (currentChannel - 1) & 1);
            idx = 0;
        }
    }

    int32_t value = readADS1234();
    Serial.printf("[CH%d | %4lu | %6lums] %d (0x%06lX)\n",
                  currentChannel, idx++, millis(), value, (uint32_t)(value & 0xFFFFFF));
>>>>>>> Stashed changes
    delay(500);
}
