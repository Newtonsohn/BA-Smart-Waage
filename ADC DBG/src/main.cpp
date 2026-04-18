#include <Arduino.h>

#define PIN_DRDY_DOUT  2   // DRDY (active low) + DOUT combined
#define PIN_SCLK       15
#define PIN_PDWN       26  // ADS1234 !PDWN, active low
#define PIN_DMS_PWR    17  // MOSFET DMS power, active low

// Reads one 24-bit two's-complement sample from ADS1234.
// Blocks until DRDY goes low (conversion ready).
int32_t readADS1234() {
    // Wait for DRDY low — no auto-reset, just wait and report status
    unsigned long t = millis();
    unsigned long lastPrint = t;
    while (digitalRead(PIN_DRDY_DOUT) == HIGH) {
        if (millis() - lastPrint > 3000) {
            Serial.printf("Waiting for DRDY... (%lus elapsed)\n", (millis() - t) / 1000);
            lastPrint = millis();
        }
    }

    // ADS1234 prefixes 2 channel-ID bits (A1, A0) before the 24-bit data.
    // With 24 clocks we capture [A1, A0, bit23..bit2]. Shift left 2 to align.
    // bit1 and bit0 become 0, max error = 3 counts — negligible for 24-bit.
    uint32_t raw = 0;
    for (int i = 0; i < 24; i++) {
        digitalWrite(PIN_SCLK, HIGH);
        delayMicroseconds(2);
        raw = (raw << 1) | digitalRead(PIN_DRDY_DOUT);
        digitalWrite(PIN_SCLK, LOW);
        delayMicroseconds(2);
    }
    raw = (raw << 2) & 0x00FFFFFF;

    // Sign-extend 24-bit -> 32-bit
    if (raw & 0x800000) raw |= 0xFF000000;
    return (int32_t)raw;
}

void setup() {
    Serial.begin(115200);

    pinMode(PIN_SCLK, OUTPUT);
    digitalWrite(PIN_SCLK, LOW);

    pinMode(PIN_DMS_PWR, OUTPUT);
    digitalWrite(PIN_DMS_PWR, HIGH); // MOSFET off initially

    pinMode(PIN_PDWN, OUTPUT);
    digitalWrite(PIN_PDWN, HIGH);    // default HIGH (running) until toggle below

    pinMode(PIN_DRDY_DOUT, INPUT);

    // Power-on sequence
    digitalWrite(PIN_DMS_PWR, LOW);  // enable DMS power (active low)
    delay(100);
    digitalWrite(PIN_PDWN, LOW);     // assert PDWN (power down) briefly
    delay(10);
    digitalWrite(PIN_PDWN, HIGH);    // release PDWN → chip starts converting
    delay(500);                      // wait for first conversion

    Serial.printf("Init done. DRDY is currently: %s\n",
                  digitalRead(PIN_DRDY_DOUT) ? "HIGH (not ready)" : "LOW (ready!)");
}

static uint32_t idx = 0;

void loop() {
    int32_t value = readADS1234();
    Serial.printf("[%4lu | %6lums] %d (0x%06lX)\n",
                  idx++, millis(), value, (uint32_t)(value & 0xFFFFFF));
    delay(500);
}
