
#pragma once
#include <EEPROM.h>

typedef struct {
  uint8_t hour;
  uint8_t minute;
  uint8_t duration;
  float   fullBrightness;
  bool    enabled;
} SettingsDataLight;

typedef struct {
  uint8_t version;
  uint8_t size;
  SettingsDataLight light1;
  SettingsDataLight light2;
} SettingsData;

const uint8_t SETTINGS_VERSION = 1;

class SettingsClass {

  public:
    SettingsData data;
    void begin() {
      reset();
      load();
    }

    void reset() {
      data.version = SETTINGS_VERSION;
      data.size = sizeof(SettingsData);
      const SettingsDataLight light = {
          7,     // hour
          15,    // minute
          30,    // duration
          1.0,   // fullBrightness
          false, // enabled
      };
      data.light1 = light;
      data.light2 = light;
    }

    void save() {
      data.version = SETTINGS_VERSION;
      data.size = sizeof(SettingsData);
      for (uint8_t i=0; i<sizeof(SettingsData); i++) {
        EEPROM.write(i, ((char*)&data)[i]);
      }
    }

  private:
    void load() {
      uint8_t version = EEPROM.read(0);
      uint8_t size = EEPROM.read(1);
      if (version != SETTINGS_VERSION || size > sizeof(SettingsData)) {
        // Version mismatch: incompatible fields.
        // Size too big: can't fit in our SettingsData field.
        return;
      }
      for (uint8_t i=2; i<size; i++) {
        ((char*)&data)[i] = EEPROM.read(i);
      }
    }
};

extern SettingsClass Settings;
