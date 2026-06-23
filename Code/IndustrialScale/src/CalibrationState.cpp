#include "CalibrationState.h"
#include "Properties.h"
#include <cmath>
#include <cstdio>
#include <Arduino.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>



#define CAL_SAMPLES            10       // averaged samples per channel per step
#define CAL_WEIGHT_G          1000.0f  // center calibration weight in grams
#define CORNER_WEIGHT_G       1000.0f  // corner trim weight in grams
#define MIN_DEFLECTION        10000.0f // minimum total deflection to accept cal
#define CAL_BUTTON_PIN        36       // GPIO36 calibration button input
#define DISPLAY_X_MARGIN      10
#define DISPLAY_Y_TITLE       20
#define DISPLAY_Y_INSTRUCTION 50
#define CAL_CHANNEL_COUNT     4
#define CAL_POINT_COUNT       5        // center + 4 corners

static volatile bool calibrationButtonInterrupt = false;


static void IRAM_ATTR onCalibrationButtonInterrupt() {
  calibrationButtonInterrupt = true;
}

static bool solveLinear4x4(double a[CAL_CHANNEL_COUNT][CAL_CHANNEL_COUNT],
                           double b[CAL_CHANNEL_COUNT],
                           double x[CAL_CHANNEL_COUNT]) {
  for (int col = 0; col < CAL_CHANNEL_COUNT; col++) {
    int pivot = col;
    double pivotAbs = fabs(a[col][col]);
    for (int row = col + 1; row < CAL_CHANNEL_COUNT; row++) {
      double candidateAbs = fabs(a[row][col]);
      if (candidateAbs > pivotAbs) {
        pivot = row;
        pivotAbs = candidateAbs;
      }
    }

    if (pivotAbs < 1e-18) {
      return false;
    }

    if (pivot != col) {
      for (int c = col; c < CAL_CHANNEL_COUNT; c++) {
        double tmp = a[col][c];
        a[col][c] = a[pivot][c];
        a[pivot][c] = tmp;
      }
      double tmpB = b[col];
      b[col] = b[pivot];
      b[pivot] = tmpB;
    }

    double diag = a[col][col];
    for (int row = col + 1; row < CAL_CHANNEL_COUNT; row++) {
      double factor = a[row][col] / diag;
      a[row][col] = 0.0;
      for (int c = col + 1; c < CAL_CHANNEL_COUNT; c++) {
        a[row][c] -= factor * a[col][c];
      }
      b[row] -= factor * b[col];
    }
  }

  for (int row = CAL_CHANNEL_COUNT - 1; row >= 0; row--) {
    double sum = b[row];
    for (int c = row + 1; c < CAL_CHANNEL_COUNT; c++) {
      sum -= a[row][c] * x[c];
    }
    x[row] = sum / a[row][row];
  }

  return true;
}

static bool solveLeastSquares(const float delta[CAL_POINT_COUNT][CAL_CHANNEL_COUNT],
                              const float target[CAL_POINT_COUNT],
                              float span[CAL_CHANNEL_COUNT]) {
  double ata[CAL_CHANNEL_COUNT][CAL_CHANNEL_COUNT] = {};
  double atb[CAL_CHANNEL_COUNT] = {};
  double solution[CAL_CHANNEL_COUNT] = {};

  for (int p = 0; p < CAL_POINT_COUNT; p++) {
    for (int r = 0; r < CAL_CHANNEL_COUNT; r++) {
      atb[r] += (double)delta[p][r] * (double)target[p];
      for (int c = 0; c < CAL_CHANNEL_COUNT; c++) {
        ata[r][c] += (double)delta[p][r] * (double)delta[p][c];
      }
    }
  }

  if (!solveLinear4x4(ata, atb, solution)) {
    return false;
  }

  for (int i = 0; i < CAL_CHANNEL_COUNT; i++) {
    span[i] = (float)solution[i];
  }
  return true;
}

static float predictWeightForDelta(const float delta[CAL_CHANNEL_COUNT],
                                   const float span[CAL_CHANNEL_COUNT]) {
  float weight = 0.0f;
  for (int i = 0; i < CAL_CHANNEL_COUNT; i++) {
    weight += span[i] * delta[i];
  }
  return weight;
}

CalibrationState::CalibrationState() {}



void CalibrationState::enter() {
  Logger::log("Enter Calibration State");

  hw = HwContext::get();
  runCalibrationZero();
}

void CalibrationState::update() {}

void CalibrationState::exit() {
  Logger::log("Exit Calibration State");
}

StateType CalibrationState::nextState() {
  return StateType::TARE_CHECK;
}

// ── helpers ───────────────────────────────────────────────────────────────────

void CalibrationState::printAllChannels() {
  int32_t v[4];
  for (int ch = 1; ch <= 4; ch++) v[ch - 1] = hw->readChannel(ch);
  char buf[80];
  snprintf(buf, sizeof(buf), "ADC  CH1:%ld  CH2:%ld  CH3:%ld  CH4:%ld",
           (long)v[0], (long)v[1], (long)v[2], (long)v[3]);
  Logger::log(buf);
}

bool CalibrationState::waitForButton(const char* msg) {
  Logger::log(msg);
  Logger::log("waiting for button press...");
  calibrationButtonInterrupt = false;
  pinMode(CAL_BUTTON_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(CAL_BUTTON_PIN), onCalibrationButtonInterrupt, FALLING);

  unsigned long lastPrint = 0;
  while (calibrationButtonInterrupt == false) {
    if (millis() - lastPrint >= 1000) {
      lastPrint = millis();
      printAllChannels();
    }
    delay(50);
  }
  detachInterrupt(digitalPinToInterrupt(CAL_BUTTON_PIN));
  return true;
}


void CalibrationState::updateCalibrationDisplay(const char* stepText, bool showArrow) {
  char instruction[40];
  if (showArrow) {
    snprintf(instruction, sizeof(instruction), "%s ->", stepText);
  } else {
    snprintf(instruction, sizeof(instruction), "%s", stepText);
  }

  // Draw to the buffer and push a partial refresh (display(true)) instead of the
  // paged full-window refresh, so the wizard steps update smoothly without the
  // full-screen flicker.
  hw->display->fillScreen(GxEPD_WHITE);
  hw->display->setTextColor(GxEPD_BLACK);

  hw->display->setFont(&FreeSansBold12pt7b);
  hw->display->setCursor(DISPLAY_X_MARGIN, DISPLAY_Y_TITLE);
  hw->display->print("Calibrating");

  hw->display->setFont(&FreeSans9pt7b);
  hw->display->setCursor(DISPLAY_X_MARGIN, DISPLAY_Y_INSTRUCTION);
  hw->display->print(instruction);

  hw->display->display(true);
}

// ── calibration sequence ──────────────────────────────────────────────────────

void CalibrationState::runCalibrationZero() {
  char buf[120];
  float calDelta[CAL_POINT_COUNT][CAL_CHANNEL_COUNT] = {};
  const float calTarget[CAL_POINT_COUNT] = {
    CAL_WEIGHT_G,
    CORNER_WEIGHT_G,
    CORNER_WEIGHT_G,
    CORNER_WEIGHT_G,
    CORNER_WEIGHT_G
  };
  const char* calLabel[CAL_POINT_COUNT] = {
    "Centered",
    "Corner 1",
    "Corner 2",
    "Corner 3",
    "Corner 4"
  };

  Logger::log("=== Calibration start ===");

  // ── Step 1: zero offset ───────────────────────────────────────────────────
  updateCalibrationDisplay("Zeroing", false);
  Logger::log("Measuring zero offset...");
  while (waitForButton("[1/3] Remove all weight from scale, then send any key.") != true) {
    delay(100);
  }

  updateCalibrationDisplay("Zeroing", true);
  for (int ch = 1; ch <= CAL_CHANNEL_COUNT; ch++) {
    Properties::zeroOffset[ch - 1] = hw->readAverage(ch, CAL_SAMPLES);
    snprintf(buf, sizeof(buf), "  CH%d zero: %ld", ch, (long)Properties::zeroOffset[ch - 1]);
    Logger::log(buf);
  }

  // ── Step 2: collect centered calibration point ────────────────────────────
  updateCalibrationDisplay("Centered", false);
  snprintf(buf, sizeof(buf),
           "[2/3] Place %.0fg weight CENTERED on scale, then send any key.", CAL_WEIGHT_G);
  Logger::log("Measuring centered calibration point...");
  while (waitForButton(buf) != true) {
    delay(100);
  }

  updateCalibrationDisplay("Centered", true);
  float totalAbsDeflection = 0.0f;
  for (int ch = 1; ch <= CAL_CHANNEL_COUNT; ch++) {
    float val = hw->readAverage(ch, CAL_SAMPLES);
    calDelta[0][ch - 1] = val - Properties::zeroOffset[ch - 1];
    totalAbsDeflection += fabsf(calDelta[0][ch - 1]);
    int sign = (calDelta[0][ch - 1] >= 0.0f) ? 1 : -1;
    snprintf(buf, sizeof(buf), "  CH%d center delta: %+ld  [%s]",
             ch, (long)calDelta[0][ch - 1], sign > 0 ? "normal" : "inverted");
    Logger::log(buf);
  }

  if (totalAbsDeflection < MIN_DEFLECTION) {
    Logger::log("ERROR: Deflection too small — check load cell wiring. Calibration aborted.");
    updateCalibrationDisplay("Error", false);
    return;
  }

  // ── Step 3: collect four corner calibration points ────────────────────────
  Logger::log("[3/3] Corner calibration: place weight over each corner one at a time.");

  for (int k = 1; k <= 4; k++) {
    int point = k;
    char cornerText[16];
    snprintf(cornerText, sizeof(cornerText), "Corner %d", k);

    updateCalibrationDisplay(cornerText, false);
    snprintf(buf, sizeof(buf),
             "  Corner %d/4: place %.0fg over corner %d, then send any key.",
             k, CORNER_WEIGHT_G, k);
    waitForButton(buf);

    updateCalibrationDisplay(cornerText, true);
    int domCh = 0;
    float maxAbsDelta = 0.0f;
    for (int ch = 1; ch <= CAL_CHANNEL_COUNT; ch++) {
      float val = hw->readAverage(ch, CAL_SAMPLES);
      calDelta[point][ch - 1] = val - Properties::zeroOffset[ch - 1];
      if (fabsf(calDelta[point][ch - 1]) > maxAbsDelta) {
        maxAbsDelta = fabsf(calDelta[point][ch - 1]);
        domCh = ch - 1;
      }
    }

    snprintf(buf, sizeof(buf), "  %s dominant CH%d", cornerText, domCh + 1);
    Logger::log(buf);
    for (int ch = 1; ch <= CAL_CHANNEL_COUNT; ch++) {
      snprintf(buf, sizeof(buf), "    CH%d delta: %+ld", ch, (long)calDelta[point][ch - 1]);
      Logger::log(buf);
    }
  }

  // ── Solve all calibration points together ─────────────────────────────────
  if (!solveLeastSquares(calDelta, calTarget, Properties::spanFactor)) {
    Logger::log("ERROR: Least-squares calibration failed — calibration matrix is singular.");
    updateCalibrationDisplay("Error", false);
    return;
  }

  Logger::log("=== Least-squares fit ===");
  for (int p = 0; p < CAL_POINT_COUNT; p++) {
    float predicted = predictWeightForDelta(calDelta[p], Properties::spanFactor);
    snprintf(buf, sizeof(buf), "  %-8s target: %.1f g  fit: %.1f g  err: %.1f g",
             calLabel[p], calTarget[p], predicted, predicted - calTarget[p]);
    Logger::log(buf);
  }

  // ── Save & summary ────────────────────────────────────────────────────────
  Properties::calibrationValid = true;
  Properties::saveConfigToNVS();

  Logger::log("=== Calibration values ===");
  Logger::log("  Ch  zeroOffset       spanFactor");
  for (int i = 0; i < CAL_CHANNEL_COUNT; i++) {
    snprintf(buf, sizeof(buf), "  %d   %12.0f    %.8f g/count",
             i + 1, Properties::zeroOffset[i], Properties::spanFactor[i]);
    Logger::log(buf);
  }
  Logger::log("=== Calibration complete, saved to NVS ===");
  updateCalibrationDisplay("Complete", false);

}



