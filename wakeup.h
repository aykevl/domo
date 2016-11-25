
#pragma once

#include "time.h"

typedef enum {
  LIGHT_OFF,
  LIGHT_WAKE,      // in minutes
  LIGHT_FASTSTART, // in seconds
  LIGHT_ON,
  LIGHT_STOP,      // in seconds
} lightState_t;

// Transition times in ms
const float LIGHT_TIME_FADE = 500.0;

class WakeupLight {
  uint8_t pin;
  lightState_t state;
  unsigned long transitionStart;
  bool wasInWakeup;
  Time time;
  uint32_t duration;

public:
  void begin(uint8_t pin);
  Time getTime() { return time; }
  uint32_t getDuration() { return duration; }
  void setWakeup(int32_t hour, int32_t minute, int32_t duration);
  void loop();
  void wake();
  void on();
  void off();
  lightState_t currentState();
  float currentBrightness();

private:
  bool inWakeup();
};

extern WakeupLight wakeup;
