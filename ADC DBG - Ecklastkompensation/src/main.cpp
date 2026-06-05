#include <Arduino.h>
#include <Preferences.h>

#define PIN_DRDY_DOUT  32
#define PIN_SCLK       15
#define PIN_PDWN       26
#define PIN_DMS_PWR    17
#define PIN_A0         4    //4
#define PIN_A1         2    //2

#define CAL_WEIGHT_G    3000.0f  // center calibration weight in grams
#define CORNER_WEIGHT_G 1000.0f  // corner trimming weight in grams
#define CAL_SAMPLES     10       // samples per channel (~2s per channel at 10SPS)

static float       zeroOffset[4];
static float       spanFactor[4];   // signed per-channel gain (g/count), encodes polarity
static int         channelSign[4];  // +1 or -1 polarity per channel (used during calibration)
static bool        calibrated = false;
static int         singleChannel = 0;   // >0 = single-channel debug mode active
static bool        singleVerbose = false; // true = bit-by-bit output
static Preferences prefs;

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
        delayMicroseconds(20);  // 20µs: DOUT needs time to settle on this PCB
        raw = (raw << 1) | digitalRead(PIN_DRDY_DOUT);
        digitalWrite(PIN_SCLK, LOW);
        delayMicroseconds(20);
    }
    // 25th clock: force DRDY high (Figure 7-10)
    digitalWrite(PIN_SCLK, HIGH); delayMicroseconds(20);
    digitalWrite(PIN_SCLK, LOW);  delayMicroseconds(20);

    if (raw & 0x800000) raw |= 0xFF000000;
    return (int32_t)raw;
}

// Switch to channel. If DRDY is still LOW from a previous conversion, force a clean
// reset via PDWN so the ADS1234 starts a fresh conversion on the new channel.
int32_t readChannel(int ch) {
    setChannel(ch);
    if (digitalRead(PIN_DRDY_DOUT) == LOW) {
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

// ── NVS persistence ───────────────────────────────────────────────────────────

void saveCalibration() {
    prefs.begin("scale", false);
    char key[8];
    for (int i = 0; i < 4; i++) {
        snprintf(key, sizeof(key), "zero%d", i);
        prefs.putFloat(key, zeroOffset[i]);
        snprintf(key, sizeof(key), "span%d", i);
        prefs.putFloat(key, spanFactor[i]);
        snprintf(key, sizeof(key), "sign%d", i);
        prefs.putInt(key, channelSign[i]);
    }
    prefs.putUChar("valid", 0xAB);
    prefs.end();
    Serial.println("Calibration saved to NVS.");
}

// Returns true if valid calibration data was found and loaded.
bool loadCalibration() {
    prefs.begin("scale", true);
    bool ok = (prefs.getUChar("valid", 0x00) == 0xAB);
    if (ok) {
        char key[8];
        for (int i = 0; i < 4; i++) {
            snprintf(key, sizeof(key), "zero%d", i);
            zeroOffset[i] = prefs.getFloat(key, 0.0f);
            snprintf(key, sizeof(key), "span%d", i);
            spanFactor[i] = prefs.getFloat(key, 0.0f);
            snprintf(key, sizeof(key), "sign%d", i);
            channelSign[i] = prefs.getInt(key, 1);
        }
        calibrated = true;
    }
    prefs.end();
    return ok;
}

void printCalibrationValues() {
    Serial.println("\n=== Calibration values (NVS) ===");
    Serial.println("  Ch  zeroOffset      spanFactor      sign");
    for (int i = 0; i < 4; i++) {
        Serial.printf("  %d   %12.2f    %14.8f    %+d\n",
                      i + 1, zeroOffset[i], spanFactor[i], channelSign[i]);
    }
    Serial.println("================================\n");
}

// ── Pin-Swap Test ─────────────────────────────────────────────────────────────

void runPinSwapTest() {
    Serial.println("\n=== Pin-Swap Test (mit aktivem ADS1234) ===");

    // Power-on: ADS1234 muss aktiv DRDY treiben fuer einen echten Test
    digitalWrite(PIN_DMS_PWR, LOW);
    delay(100);
    digitalWrite(PIN_PDWN, LOW); delay(10); digitalWrite(PIN_PDWN, HIGH);
    Serial.println("Power-on. Warte auf DRDY LOW (ADS1234 treibt aktiv)...");
    unsigned long t0 = millis();
    while (digitalRead(PIN_DRDY_DOUT) == HIGH && millis() - t0 < 5000) delay(5);
    if (millis() - t0 >= 5000) {
        Serial.println("FAIL: DRDY nie LOW. Pruefe DMS_PWR und PDWN Verdrahtung.");
        while (true) delay(1000);
    }
    Serial.printf("DRDY LOW nach %lums.\n\n", millis() - t0);

    // Test 1: DRDY ist LOW (ADS1234 treibt LOW) — GPIO2 auf HIGH setzen
    // Wenn GPIO32 HIGH liest -> kurzgeschlossen mit GPIO2
    // Wenn GPIO32 LOW bleibt -> ADS1234 gewinnt, keine Verbindung
    digitalWrite(PIN_A1, HIGH);
    delayMicroseconds(100);
    int rd1 = digitalRead(PIN_DRDY_DOUT);
    Serial.printf("[Test 1] ADS1234 treibt DRDY=LOW, A1 (GPIO%d) auf HIGH gesetzt\n", PIN_A1);
    Serial.printf("         GPIO%d liest: %s  (erwartet LOW wenn kein Kurzschluss)\n",
                  PIN_DRDY_DOUT, rd1 ? "HIGH" : "LOW");
    digitalWrite(PIN_A1, LOW);
    delay(10);

    // Test 2: DRDY auf HIGH bringen (25 Takte senden), dann GPIO2 LOW setzen
    // Warten bis DRDY LOW, dann 25 Takte:
    while (digitalRead(PIN_DRDY_DOUT) == HIGH) delay(1);
    for (int i = 0; i < 25; i++) {
        digitalWrite(PIN_SCLK, HIGH); delayMicroseconds(2);
        digitalWrite(PIN_SCLK, LOW);  delayMicroseconds(2);
    }
    // DRDY sollte jetzt HIGH sein (nach 25. Takt)
    delayMicroseconds(100);
    int drdyNow = digitalRead(PIN_DRDY_DOUT);
    Serial.printf("\n[Test 2] Nach 25 SCLK-Takten: GPIO%d = %s  (erwartet HIGH)\n",
                  PIN_DRDY_DOUT, drdyNow ? "HIGH" : "LOW");
    digitalWrite(PIN_A1, LOW);
    delayMicroseconds(100);
    int rd2 = digitalRead(PIN_DRDY_DOUT);
    Serial.printf("         ADS1234 treibt DRDY=%s, A1 (GPIO%d) auf LOW gesetzt\n",
                  drdyNow ? "HIGH" : "LOW", PIN_A1);
    Serial.printf("         GPIO%d liest: %s  (erwartet %s wenn kein Kurzschluss)\n",
                  PIN_DRDY_DOUT, rd2 ? "HIGH" : "LOW", drdyNow ? "HIGH" : "LOW");
    digitalWrite(PIN_A1, LOW);

    // Auswertung Pin-Swap
    Serial.println();
    bool shortDetected = (rd1 == HIGH) || (rd2 == LOW && drdyNow == HIGH);
    if (shortDetected) {
        Serial.println(">>> KURZSCHLUSS GPIO2/GPIO32 BESTAETIGT (mit aktivem ADS1234) <<<");
    } else {
        Serial.println("Kein Kurzschluss zwischen GPIO2 und GPIO32.");
    }

    // SCLK-Response Test: sehen ob DOUT auf SCLK reagiert
    Serial.println("\n=== SCLK-Response Test ===");
    Serial.println("Warte auf DRDY LOW...");
    while (digitalRead(PIN_DRDY_DOUT) == HIGH) delay(1);
    Serial.printf("DRDY LOW. DOUT ohne SCLK: %d\n", digitalRead(PIN_DRDY_DOUT));
    Serial.println("Sende 24 SCLK-Pulse, lese DOUT nach jeder steigenden Flanke:");
    Serial.print("Bits: ");
    for (int i = 0; i < 24; i++) {
        digitalWrite(PIN_SCLK, HIGH); delayMicroseconds(2);
        Serial.print(digitalRead(PIN_DRDY_DOUT));
        digitalWrite(PIN_SCLK, LOW);  delayMicroseconds(2);
    }
    // 25th clock
    digitalWrite(PIN_SCLK, HIGH); delayMicroseconds(20);
    digitalWrite(PIN_SCLK, LOW);  delayMicroseconds(20);
    Serial.printf("\nNach 25. Takt: DOUT=%d  (erwartet HIGH)\n", digitalRead(PIN_DRDY_DOUT));

    Serial.println("\nReset druecken.");
    while (true) delay(1000);
}

// ── Debug ─────────────────────────────────────────────────────────────────────

void runDebug() {
    Serial.println("\n=== ADC Debug ===");
    Serial.printf("  PIN_DRDY_DOUT = %d\n", PIN_DRDY_DOUT);
    Serial.printf("  PIN_SCLK      = %d\n", PIN_SCLK);
    Serial.printf("  PIN_PDWN      = %d\n", PIN_PDWN);
    Serial.printf("  PIN_DMS_PWR   = %d\n", PIN_DMS_PWR);
    Serial.printf("  PIN_A0        = %d\n", PIN_A0);
    Serial.printf("  PIN_A1        = %d\n", PIN_A1);
    if (PIN_A0 == 2 || PIN_A1 == 2 || PIN_DRDY_DOUT == 2) {
        Serial.println("  WARNING: GPIO2 ist ein Strapping-Pin (muss beim Boot LOW sein).");
        Serial.println("           Als Output nach dem Boot OK, aber kein Pull-up auf dem PCB!");
    }

    Serial.printf("\nVor Power-on: DRDY=%d  SCLK=%d\n",
                  digitalRead(PIN_DRDY_DOUT), digitalRead(PIN_SCLK));

    // Power-on sequence
    digitalWrite(PIN_DMS_PWR, LOW);
    delay(100);
    digitalWrite(PIN_PDWN, LOW);
    delay(10);
    digitalWrite(PIN_PDWN, HIGH);
    Serial.println("Power-on Puls fertig. Warte bis zu 5s auf DRDY LOW...");

    unsigned long t0 = millis();
    bool drdyReady = false;
    while (millis() - t0 < 5000) {
        if (digitalRead(PIN_DRDY_DOUT) == LOW) { drdyReady = true; break; }
        delay(10);
    }

    if (!drdyReady) {
        Serial.println("FAIL: DRDY wurde nie LOW in 5s.");
        Serial.println("  Pruefe: DMS_PWR-Leitung, PDWN-Leitung, DRDY-Pin-Zuweisung.");
        return;
    }
    Serial.printf("OK: DRDY nach %lums LOW\n", millis() - t0);

    // Read one raw value per channel and show A0/A1 states
    Serial.println("\nCh  A0  A1  DRDY_vor_Lesen  Rohwert");
    for (int ch = 1; ch <= 4; ch++) {
        setChannel(ch);
        if (ch == 1 && digitalRead(PIN_DRDY_DOUT) == LOW) {
            digitalWrite(PIN_PDWN, LOW);
            delayMicroseconds(500);
            digitalWrite(PIN_PDWN, HIGH);
        }
        int a0   = digitalRead(PIN_A0);
        int a1   = digitalRead(PIN_A1);
        int drdy = digitalRead(PIN_DRDY_DOUT);

        unsigned long tw = millis();
        while (digitalRead(PIN_DRDY_DOUT) == HIGH && millis() - tw < 2000) delay(1);
        if (millis() - tw >= 2000) {
            Serial.printf(" %d   %d   %d   %d               TIMEOUT — DRDY blieb HIGH\n",
                          ch, a0, a1, drdy);
            continue;
        }

        uint32_t raw = 0;
        for (int i = 0; i < 24; i++) {
            digitalWrite(PIN_SCLK, HIGH); delayMicroseconds(20);
            raw = (raw << 1) | digitalRead(PIN_DRDY_DOUT);
            digitalWrite(PIN_SCLK, LOW);  delayMicroseconds(20);
        }
        digitalWrite(PIN_SCLK, HIGH); delayMicroseconds(20);
        digitalWrite(PIN_SCLK, LOW);  delayMicroseconds(20);
        if (raw & 0x800000) raw |= 0xFF000000;

        Serial.printf(" %d   %d   %d   %d               %d\n",
                      ch, a0, a1, drdy, (int32_t)raw);
    }
    Serial.println("\nDebug fertig. Reset druecken.");
    while (true) delay(1000);
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
        snprintf(buf, sizeof(buf), "  Corner %d/4: place %.0fg over corner %d, press any key...", k, CORNER_WEIGHT_G, k);
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
            spanFactor[domCh] = (CORNER_WEIGHT_G - sumOther) / delta[domCh];
            Serial.printf("  CH%d span adjusted to %.8f g/count\n", domCh + 1, spanFactor[domCh]);
        } else {
            Serial.printf("  CH%d: deflection too small, skipped.\n", domCh + 1);
        }
    }

    saveCalibration();
    printCalibrationValues();
    Serial.println("Calibration complete — starting measurement.\n");
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
    // DRDY LOW = Messung fertig, jetzt auslesen.  HIGH = Wandlung laeuft, warten.

    bool hasNVS = loadCalibration();
    if (hasNVS) {
        Serial.println("  w  =  Gewicht anzeigen (NVS-Kalibrierung)");
        Serial.println("  c  =  neu kalibrieren");
    } else {
        Serial.println("  c  =  kalibrieren und Gewicht anzeigen");
    }
    Serial.println("  r  =  rohe ADC-Werte alle 4 Kanaele");
    Serial.println("  s  =  einzelner Kanal (mit DRDY-Anzeige)");
    Serial.println("  d  =  ADC Pins debuggen");
    Serial.println("  p  =  Pin-Swap Test (DRDY vs A1 Leiterbahn)");
    Serial.println();
    // DRDY/DOUT ist ein Dual-Purpose Pin:
    //   HIGH  = ADS1234 konvertiert gerade  → warten, noch nicht lesen
    //   LOW   = Konvertierung fertig        → jetzt 24 Bits auslesen
    // Nach dem 25. SCLK-Puls geht DRDY wieder HIGH und die naechste Konvertierung startet.
    // Das Lesen passiert auf der steigenden SCLK-Flanke waehrend DRDY/DOUT LOW ist.
    Serial.println("  (DRDY HIGH=busy, LOW=ready. Lesen nur wenn LOW.)");
    Serial.print("Choice: ");

    char mode = 0;
    while (mode != 'r' && mode != 'c' && mode != 'w' && mode != 'd' && mode != 's' && mode != 'p') {
        if (Serial.available()) mode = Serial.read();
        if (mode == 'w' && !hasNVS) mode = 0;
        delay(50);
    }
    Serial.println(mode);

    if (mode == 'd') {
        runDebug();
        return;
    }

    if (mode == 'p') {
        runPinSwapTest();
        return;
    }

    if (mode == 's') {
        Serial.print("Kanal waehlen (1-4): ");
        char ch = 0;
        while (ch < '1' || ch > '4') {
            if (Serial.available()) ch = Serial.read();
            delay(50);
        }
        Serial.println(ch);
        singleChannel = ch - '0';
        calibrated = false;

        Serial.print("Ausgabe-Modus (n=normal, v=verbose Bits): ");
        char vm = 0;
        while (vm != 'n' && vm != 'v') {
            if (Serial.available()) vm = Serial.read();
            delay(50);
        }
        Serial.println(vm);
        singleVerbose = (vm == 'v');
    }

    // Power-on sequence (after mode selected so DMS is fresh)
    digitalWrite(PIN_DMS_PWR, LOW);
    delay(100);
    digitalWrite(PIN_PDWN, LOW);
    delay(10);
    digitalWrite(PIN_PDWN, HIGH);
    delay(500);

    Serial.printf("ADC ready. DRDY: %s\n",
                  digitalRead(PIN_DRDY_DOUT) ? "HIGH (not ready)" : "LOW (ready)");

    if (mode == 'r') {
        calibrated = false;  // Rohwerte erzwingen, auch wenn NVS geladen wurde
    } else if (mode == 'c') {
        runCalibration();
    } else if (mode == 'w') {
        printCalibrationValues();
        Serial.println("Gewicht wird angezeigt (NVS-Kalibrierung).\n");
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

    // ── Single-channel debug mode ──────────────────────────────────────────────
    if (singleChannel > 0) {
        setChannel(singleChannel);
        if (singleChannel == 1 && digitalRead(PIN_DRDY_DOUT) == LOW) {
            digitalWrite(PIN_PDWN, LOW);
            delayMicroseconds(500);
            digitalWrite(PIN_PDWN, HIGH);
        }
        int a0             = digitalRead(PIN_A0);
        int a1             = digitalRead(PIN_A1);
        int drdyAfterSwitch = digitalRead(PIN_DRDY_DOUT);  // sofort nach Kanalwechsel

        unsigned long tw = millis();
        while (digitalRead(PIN_DRDY_DOUT) == HIGH && millis() - tw < 2000) delay(1);
        unsigned long waited = millis() - tw;
        int drdyAfterWait   = digitalRead(PIN_DRDY_DOUT);  // nach dem Warten (soll LOW sein)

        if (waited >= 2000) {
            Serial.printf("CH%d  A0=%d A1=%d  DRDY: switch=%s wait=TIMEOUT\n",
                          singleChannel, a0, a1,
                          drdyAfterSwitch ? "HIGH" : "LOW");
        } else {
            uint32_t raw = 0;
            char bits[25];
            for (int i = 0; i < 24; i++) {
                digitalWrite(PIN_SCLK, HIGH); delayMicroseconds(20);
                int bit = digitalRead(PIN_DRDY_DOUT);
                bits[i] = '0' + bit;
                raw = (raw << 1) | bit;
                digitalWrite(PIN_SCLK, LOW);  delayMicroseconds(20);
            }
            bits[24] = '\0';
            digitalWrite(PIN_SCLK, HIGH); delayMicroseconds(20);  // 25th clock
            digitalWrite(PIN_SCLK, LOW);  delayMicroseconds(20); // 20µs: langsamer Anstieg auf neuer PCB
            int drdyAfter25 = digitalRead(PIN_DRDY_DOUT);        // nach 25. Takt (soll HIGH sein)
            if (raw & 0x800000) raw |= 0xFF000000;

            if (singleVerbose) {
                Serial.printf("CH%d  A0=%d A1=%d  DRDY: switch=%s wait=%s(%lums) after25=%s  bits: %s  raw: %d\n",
                              singleChannel, a0, a1,
                              drdyAfterSwitch ? "HIGH" : "LOW",
                              drdyAfterWait   ? "HIGH" : "LOW", waited,
                              drdyAfter25     ? "HIGH" : "LOW",
                              bits, (int32_t)raw);
            } else {
                Serial.printf("CH%d  A0=%d A1=%d  DRDY: switch=%s wait=%s(%lums) after25=%s  raw: %d\n",
                              singleChannel, a0, a1,
                              drdyAfterSwitch ? "HIGH" : "LOW",
                              drdyAfterWait   ? "HIGH" : "LOW", waited,
                              drdyAfter25     ? "HIGH" : "LOW",
                              (int32_t)raw);
            }
        }
        delay(2000);
        return;
    }

    // ── Normal multi-channel read ──────────────────────────────────────────────
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
