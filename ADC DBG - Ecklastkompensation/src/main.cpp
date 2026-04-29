#include <Arduino.h>

#define PIN_DRDY_DOUT  2
#define PIN_SCLK       15
#define PIN_PDWN       26
#define PIN_DMS_PWR    17
#define PIN_A0         32
#define PIN_A1         4

#define CAL_WEIGHT_G   3000.0f  // known calibration weight in grams
#define CAL_SAMPLES    5        // samples per channel (~2s per channel at 10SPS)

static float zeroOffset[4];
static float spanFactor[4];     // signed per-channel gain (g/count), encodes polarity
static int   channelSign[4];    // +1 or -1 polarity per channel (used during calibration)
static bool  calibrated = false;

// ── ADS1234 low-level ─────────────────────────────────────────────────────────

void setChannel(int ch) {
    if (ch < 1 || ch > 4) return;
    int i = ch - 1;
    digitalWrite(PIN_A0, i & 0x01);
    digitalWrite(PIN_A1, (i >> 1) & 0x01);
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
    char buf[100];

    // Step 1: zero offset
    waitForSerial("\n[Step 1/3] Remove ALL weight from scale.\nPress any key when ready...");
    Serial.printf("Measuring zero offset (~%ds)...\n", estimatedSeconds);
    for (int ch = 1; ch <= 4; ch++) {
        zeroOffset[ch - 1] = readAverage(ch, CAL_SAMPLES);
        Serial.printf("  CH%d: %.0f\n", ch, zeroOffset[ch - 1]);
    }

    // Step 2: centered calibration — all channels get equal initial span factor
    snprintf(buf, sizeof(buf), "\n[Step 2/3] Place %.0fg weight CENTERED on scale.\nPress any key when ready...", CAL_WEIGHT_G);
    waitForSerial(buf);
    Serial.printf("Measuring %.0fg centered (~%ds)...\n", CAL_WEIGHT_G, estimatedSeconds);

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
        return;
    }

    float initScale = CAL_WEIGHT_G / totalAbsDeflection;
    for (int i = 0; i < 4; i++) {
        spanFactor[i] = channelSign[i] * initScale;
    }
    calibrated = true;
    Serial.printf("Initial scale factor: %.6f g/count\n", initScale);

    // Step 3: corner trimming
    // Place weight over each load cell corner in turn.
    // The most-deflected channel at each corner gets its span factor adjusted
    // so the total reads exactly CAL_WEIGHT_G. One pass is sufficient for
    // typical DMS mismatches (<5%).
    Serial.println("\n[Step 3/3] Corner trim: place weight over each load cell one at a time.");
    for (int k = 1; k <= 4; k++) {
        snprintf(buf, sizeof(buf), "  Corner %d/4: place %.0fg over corner %d, press any key...", k, CAL_WEIGHT_G, k);
        waitForSerial(buf);

        float delta[4];
        int domCh = 0;
        float maxAbsDelta = 0;
        for (int ch = 1; ch <= 4; ch++) {
            float val = readAverage(ch, CAL_SAMPLES);
            delta[ch - 1] = val - zeroOffset[ch - 1];
            if (fabsf(delta[ch - 1]) > maxAbsDelta) {
                maxAbsDelta = fabsf(delta[ch - 1]);
                domCh = ch - 1;
            }
        }

        float measured = 0;
        for (int i = 0; i < 4; i++) {
            measured += spanFactor[i] * delta[i];
        }
        Serial.printf("  Reading: %.1f g  (dominant CH%d)\n", measured, domCh + 1);

        if (maxAbsDelta > 1000) {
            // Solve exactly for the dominant channel's span factor so that
            // sum_other + spanFactor[domCh] * delta[domCh] = CAL_WEIGHT_G
            float sumOther = measured - spanFactor[domCh] * delta[domCh];
            spanFactor[domCh] = (CAL_WEIGHT_G - sumOther) / delta[domCh];
            Serial.printf("  CH%d span adjusted to %.6f g/count\n", domCh + 1, spanFactor[domCh]);
        } else {
            Serial.printf("  CH%d: deflection too small, skipped.\n", domCh + 1);
        }
    }

    Serial.println("\nCalibration complete!\n");
}

// ── Setup ─────────────────────────────────────────────────────────────────────

void setup() {
    Serial.begin(115200);

    pinMode(PIN_SCLK, OUTPUT);    digitalWrite(PIN_SCLK, LOW);
    pinMode(PIN_DMS_PWR, OUTPUT); digitalWrite(PIN_DMS_PWR, HIGH);
    pinMode(PIN_PDWN, OUTPUT);    digitalWrite(PIN_PDWN, HIGH);
    pinMode(PIN_DRDY_DOUT, INPUT);
    pinMode(PIN_A0, OUTPUT);
    pinMode(PIN_A1, OUTPUT);
    setChannel(1);

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
    delay(100);
    digitalWrite(PIN_PDWN, LOW);
    delay(10);
    digitalWrite(PIN_PDWN, HIGH);
    delay(500);

    Serial.printf("ADC ready. DRDY: %s\n",
                  digitalRead(PIN_DRDY_DOUT) ? "HIGH (not ready)" : "LOW (ready)");

    if (mode == 'c') {
        runCalibration();
    }
}

// ── Loop ──────────────────────────────────────────────────────────────────────

void loop() {
    if (Serial.available()) {
        char c = Serial.read();
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
        float weight = 0;
        for (int ch = 0; ch < 4; ch++) {
            weight += spanFactor[ch] * (values[ch] - zeroOffset[ch]);
        }
        Serial.printf("%.1f g\n", weight);
    } else {
        Serial.printf("CH1:%d CH2:%d CH3:%d CH4:%d\n",
                      values[0], values[1], values[2], values[3]);
    }

    delay(500);
}
