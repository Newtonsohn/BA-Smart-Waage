#include "FailureState.h"
#include "Properties.h"
#include "StateType.h"
#include <QRCodeGFX.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSansBold12pt7b.h>

#define MAC_ADDRESS_LENGTH 6
#define MAC_STRING_LENGTH 18
#define URL_BUFFER_LENGTH 128

#define DISPLAY_X_MARGIN 10
#define DISPLAY_Y_TITLE 30
#define DISPLAY_Y_MAC 55
#define DISPLAY_Y_ERROR 110
#define DISPLAY_X_QRCODE 200
#define DISPLAY_Y_QRCODE 10
#define QR_CODE_SCALE 2

FailureState::FailureState()
  : hw(HwContext::get()) {}

void FailureState::enter() {
  Logger::log("Enter Failure State");

  updateDisplayWithFailure();

  // Send Device to sleep indefinitly
  esp_deep_sleep_start();
}

void FailureState::update() {
  // Stump methode never called due to deep sleep initiating a fresh boot after the timer ends
  Logger::log("Warning: exit() should never be called in FailureState");
}

void FailureState::exit() {
  // Stump methode never called due to deep sleep initiating a fresh boot after the timer ends
  Logger::log("Warning: exit() should never be called in FailureState");
}

StateType FailureState::nextState() {
  // Stump methode never called due to deep sleep initiating a fresh boot after the timer ends
  Logger::log("Warning: nexState() should never be called in FailureState");
  return StateType::FAILURE;
}

void FailureState::updateDisplayWithFailure() {
  // Create an array to store the MAC address
  uint8_t btMac[MAC_ADDRESS_LENGTH];
  memcpy(btMac, Properties::bleMacAddress, sizeof(btMac));

  // Convert the MAC address to a string
  char macStr[MAC_STRING_LENGTH];
  char url[URL_BUFFER_LENGTH];

  //Convert MAC-Address to display as text and within barcode
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X", btMac[0], btMac[1], btMac[2], btMac[3], btMac[4], btMac[5]);
  snprintf(
    url,
    sizeof(url),
    "%s%02X%02X%02X%02X%02X%02X",
    Properties::BACKEND_URL,
    btMac[0], btMac[1], btMac[2], btMac[3], btMac[4], btMac[5]);

  // Initialize the display
  hw->display->fillScreen(GxEPD_WHITE);

  // Write title
  hw->display->setFont(&FreeSansBold12pt7b);
  hw->display->setCursor(DISPLAY_X_MARGIN, DISPLAY_Y_TITLE);
  hw->display->setTextColor(GxEPD_BLACK);
  hw->display->print("BLE Address: ");
  hw->display->setFont(&FreeSans9pt7b);
  hw->display->setCursor(DISPLAY_X_MARGIN, DISPLAY_Y_MAC);
  hw->display->print(macStr);

  // Failure Message
  hw->display->setCursor(DISPLAY_X_MARGIN, DISPLAY_Y_ERROR);
  hw->display->setFont(&FreeSans9pt7b);

  hw->display->print(Properties::failureMessage);

  // Display QR-Code
  QRCodeGFX qrcode(*hw->display);
  qrcode.setScale(QR_CODE_SCALE);
  qrcode.draw(url, DISPLAY_X_QRCODE, DISPLAY_Y_QRCODE);

  // Update display
  hw->display->display(true);
}
