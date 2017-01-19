
#pragma once

#include <ArduinoJson.h>

#include "time.h"

typedef enum {
  LIGHT_UNDEFINED,
  LIGHT_OFF,
  LIGHT_SWITCH,
  LIGHT_WAKE,
  LIGHT_ON,
} lightState_t;

// Transition times in ms
const float LIGHT_TIME_FADE = 500.0;

class WakeupLight {
  uint8_t pin;
  lightState_t state;
  lightState_t nextState;
  unsigned long transitionStart;
  bool wasInWakeup;
  Time time;
  uint32_t duration;
  bool enabled;
  float fullBrightness;

public:
  void begin(uint8_t pin);
  Time getTime() { return time; }
  uint32_t getDuration() { return duration; }
  void setWakeup(int32_t hour, int32_t minute, int32_t duration);
  void setWakeup(int32_t dayTime, int32_t duration);
  void loop();
  void off();
  void wake();
  void on();
  lightState_t currentState();
  float currentBrightness();
  void recvState(JsonObject &value);

private:
  bool inWakeup();
  void sendState();
  void _off();
  void _wake();
  void _on();
};

extern WakeupLight wakeup;
