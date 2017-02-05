
#pragma once

#include <Arduino.h>
#include "time.h"
#include "settings.h"

typedef enum {
  LIGHT_UNDEFINED,
  LIGHT_OFF,
  LIGHT_WAKE,
  LIGHT_ON,
  LIGHT_SWITCH,
} lightState_t;

const uint8_t LIGHT_FLAG_ENABLED     = 0b10000000;
const uint8_t LIGHT_FLAG_STATUS_MASK = 0b00000011;


// Transition times in ms
const float LIGHT_TIME_FADE = 500.0;

class Light {
  uint8_t pin;
  uint8_t child;
  SettingsDataLight *settings;
  lightState_t state;
  lightState_t nextState;
  unsigned long transitionStart;
  bool wasInWakeup;
  Time time;
  uint32_t duration; // duration in ms
  bool enabled;
  float fullBrightness;

public:
  void begin(uint8_t pin, uint8_t child, SettingsDataLight *settings);
  Time getTime() { return time; }
  uint32_t getDuration() { return duration; }
  void setWakeup(int32_t hour, int32_t minute, int32_t duration, bool enabled);
  void loop();
  void setState(lightState_t newState);
  void off();
  void wake();
  void on();
  lightState_t currentState();
  float currentBrightness();
  void gotMessage(uint8_t *arg);
  void sendState();

private:
  bool inWakeup();
  void _off();
  void _wake();
  void _on();
};
