
/* Communication with the actual amplifier.
 *
 */

#include <Arduino.h>
#include <Wire.h>

#include "amplifier.h"
#include "settings.h"
#include "config.h"
#include "rotary.h"
#include "mqtt.h"
#include "button.h"

const uint8_t AMPLIFIER_EEPROM_ADDRESS = 4; // skip the first 4 bytes
const unsigned long AMPLIFIER_EEPROM_TIMEOUT = 1000; // wait 1s before writing the volume
const uint8_t AMPLIFIER_BUTTON_PIN = D1;
const uint8_t AMPLIFIER_LED_PIN = D0;
const uint8_t AMPLIFIER_SDA = D2;
const uint8_t AMPLIFIER_SCL = D3;

Button amplifierButton(AMPLIFIER_BUTTON_PIN);

// 0x4b, address with both ADDR ports high (from the datasheet).
#define AMPLIFIER_ADDRESS 0b1001011

uint8_t amplifierVolume = 40;

bool amplifierMuted() {
  return amplifierVolume & 0x80;
}

// private
void amplifierWriteVolume() {
  digitalWrite(AMPLIFIER_LED_PIN, !amplifierMuted());

  Wire.beginTransmission(AMPLIFIER_ADDRESS);
  if (amplifierMuted()) {
    Wire.write(0);
  } else {
    Wire.write(amplifierVolume);
  }
  Wire.endTransmission();
}

void amplifierMute(bool muted) {
  if (muted == amplifierMuted()) {
    // no change
    return;
  }

  if (muted) {
    amplifierVolume = amplifierVolume | 0x80; // set bit
  } else {
    amplifierVolume = amplifierVolume & ~0x80; // clear bit
  }
  digitalWrite(AMPLIFIER_LED_PIN, !muted);
  amplifierWriteVolume();
  amplifierSendState();
}

// Increase or decrease volume.
void amplifierChangeVolume(int8_t change) {
  if (amplifierMuted() || !change) {
    // nothing to do
    return;
  }

  // Set new volume with cutoff.
  int8_t newVolume = (int8_t)amplifierVolume + (int8_t)change;
  if (newVolume > 63) {
    newVolume = 63;
  }
  if (newVolume < 0) {
    newVolume = 0;
  }
  amplifierVolume = (uint8_t)newVolume;
  amplifierSendState();
  amplifierWriteVolume();

  log(String("amp volume: ") + amplifierVolume);
}

void amplifierSetup() {
  // Rotary buttons and serial use the same pins, so disable rotary when serial is enabled.
#ifndef SERIAL_ENABLED
  rotarySetup();
#endif
  Wire.begin(AMPLIFIER_SDA, AMPLIFIER_SCL);
  pinMode(AMPLIFIER_LED_PIN, OUTPUT);

  // Initialize volume
  if (Settings.data.amp_volume != amplifierVolume) {
    amplifierVolume = Settings.data.amp_volume;
    amplifierWriteVolume();
  }
}

void amplifierLoop() {
  amplifierButton.loop();
  static bool buttonWasPressed = false;
  bool buttonPressed = amplifierButton.pressed();
  if (buttonPressed && !buttonWasPressed) {
    amplifierMute(!amplifierMuted());
  }
  buttonWasPressed = buttonPressed;

  static int8_t counter = 0;
#ifndef SERIAL_ENABLED
  counter += rotaryRead();
#endif
  if (counter >= 4) {
    counter -= 4;
    amplifierChangeVolume(1);
  } else if (counter <= -4) {
    counter += 4;
    amplifierChangeVolume(-1);
  }

  static unsigned long lastMillis = 0;
  unsigned long currentMillis = millis();
  if (currentMillis - lastMillis > AMPLIFIER_EEPROM_TIMEOUT) {
    lastMillis = currentMillis;
    if (amplifierVolume != Settings.data.amp_volume) {
      Settings.data.amp_volume = amplifierVolume;
      Settings.save();
    }
  }
}

void amplifierRecvState(JsonObject &value) {
  uint8_t volume = int8_t(float(value["volume"]) * 63.0 + 0.5);
  bool enabled = value["enabled"];
  uint8_t newVolume = (enabled ? 0x00 : 0x80) | volume;
  if (newVolume != amplifierVolume) {
    amplifierVolume = newVolume;
    amplifierWriteVolume();
  }
}

void amplifierSendState() {
  if (!mqtt.connected()) return;

  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["origin"] = CLIENT_ID;
  JsonObject& values = root.createNestedObject("value");

  values["volume"] = float(amplifierVolume & 0x3f) / 63.0;
  values["enabled"] = !amplifierMuted();

  const size_t messageMaxLen = 128;
  uint8_t message[messageMaxLen];
  size_t messageLen = root.printTo((char*)message, messageMaxLen);

  mqtt.publish(MQTT_PREFIX "a/amplifier", message, messageLen, true);
}
