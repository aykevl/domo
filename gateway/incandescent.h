
#pragma once
#include <Arduino.h>

/* Emulate an incandescent light bulb.
 */

typedef enum {
  STATE_OFF,
  STATE_STARTING, // turning on
  STATE_ON,
  STATE_STOPPING, // turning off
} IncandescentState;

const float INCANDESCENT_TRANSITION_TIME = 500.0;

#ifdef PWMRANGE
#define INCANDESCENT_PWMRANGE PWMRANGE
#else
#define INCANDESCENT_PWMRANGE 255
#endif

class Incandescent {
  uint8_t pin;
  IncandescentState state;
  unsigned long transitionStart;

public:
  Incandescent(uint8_t pin);
  inline IncandescentState getState() const;
  void loop();
  void turnOn();
  void turnOff();
  void set(bool on);

private:
  float decayOn(unsigned long time) const;
  float decayOff(unsigned long time) const;
  unsigned long reverseDecayOn(float y) const;
  unsigned long reverseDecayOff(float y) const;
};
