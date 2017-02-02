
#pragma once

#include <EEPROM.h>

typedef struct {
  uint8_t version;
  uint8_t size;
  uint8_t wake_hour;
  uint8_t wake_minute;
  uint8_t wake_duration;
  float   wake_fullBrightness;
  bool    wake_enabled;
} SettingsData;

const uint8_t SETTINGS_VERSION = 1;

class SettingsClass {

  public:
    SettingsData data;
    void begin() {
      EEPROM.begin(sizeof(SettingsData));
      reset();
      load();
    }

    void reset() {
      data.version = SETTINGS_VERSION;
      data.size = sizeof(SettingsData);
      data.wake_hour = 7;
      data.wake_minute = 15;
      data.wake_duration = 30;
      data.wake_fullBrightness = 1.0;
      data.wake_enabled = false;
    }

    void save() {
      data.version = SETTINGS_VERSION;
      data.size = sizeof(SettingsData);
      for (uint8_t i=0; i<sizeof(SettingsData); i++) {
        EEPROM.write(i, ((char*)&data)[i]);
      }
      EEPROM.commit();
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
