
/* Communication with the actual amplifier.
 *
 */

#include <Arduino.h>
#include <Wire.h>

#include "amplifier.h"
#include "settings.h"
#include "rotary.h"
#include "mqtt.h"

const uint8_t AMPLIFIER_EEPROM_ADDRESS = 4; // skip the first 4 bytes
const unsigned long AMPLIFIER_EEPROM_TIMEOUT = 1000; // wait 1s before writing the volume

// 0x4b, address with both ADDR ports high (from the datasheet).
#define AMPLIFIER_ADDRESS 0b1001011

uint8_t amplifierVolume = 40;

bool amplifierMuted() {
  return amplifierVolume & 0x80;
}

// private
void amplifierWriteVolume() {
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
  amplifierWriteVolume();
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
  amplifierWriteVolume();

  log(String("amp volume: ") + amplifierVolume);
}

void amplifierSetup() {
  rotarySetup();
  if (Settings.data.amp_volume != amplifierVolume) {
    amplifierVolume = Settings.data.amp_volume;
    amplifierWriteVolume();
  }
  Wire.begin(D2, D3);
}

void amplifierLoop() {
  static int8_t counter = 0;
  counter += rotaryRead();
  if (counter >= 2) {
    counter -= 2;
    amplifierChangeVolume(1);
  } else if (counter <= -2) {
    counter += 2;
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
