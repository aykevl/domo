
#include "incandescent.h"

Incandescent::Incandescent(uint8_t pin) {
  this->pin = pin;
  this->state = STATE_OFF;
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}

IncandescentState Incandescent::getState() const {
  return state;
}

void Incandescent::loop() {
  switch (state) {
    case STATE_STARTING: {
      float brightness = decayOn(millis()-transitionStart);
      int value = (int)(brightness*(INCANDESCENT_PWMRANGE+1));
      if (value < INCANDESCENT_PWMRANGE) {
        analogWrite(pin, value);
      } else {
        // light fully on
        state = STATE_ON;
        digitalWrite(pin, HIGH);
      }
      break;
    }
    case STATE_STOPPING: {
      float brightness = decayOff(millis() - transitionStart);
      int value = (int)(brightness*(INCANDESCENT_PWMRANGE+1));
      if (value > 0) {
        analogWrite(pin, value);
      } else {
        // light fully off
        state = STATE_OFF;
        digitalWrite(pin, LOW);
      }
      break;
    }
    default:
      // do nothing
      break;
  }
}

void Incandescent::turnOn() {
  switch (state) {
    case STATE_STARTING:
    case STATE_ON:
      // do nothing
      break;
    case STATE_STOPPING: {
      // Calculate where we are in the turn-off sequence and set
      // transitionStart accordingly.
      state = STATE_STARTING;
      unsigned long currentMillis = millis();
      float y = decayOff(currentMillis - transitionStart);
      transitionStart = currentMillis - reverseDecayOn(y);
      loop();
      break;
    }
    default: // STATE_OFF
      state = STATE_STARTING;
      transitionStart = millis();
      loop();
      break;
  }
}

void Incandescent::turnOff() {
  switch (state) {
    case STATE_STOPPING:
    case STATE_OFF:
      // do nothing
      break;
    case STATE_STARTING: {
      // Calculate where we are in the turn-on sequence and set
      // transitionStart accordingly.
      state = STATE_STOPPING;
      unsigned long currentMillis = millis();
      float y = decayOn(currentMillis - transitionStart);
      transitionStart = currentMillis - reverseDecayOff(y);
      loop();
      break;
    }
    default: // STATE_ON
      state = STATE_STOPPING;
      transitionStart = millis();
      loop();
      break;
  }
}

void Incandescent::set(bool on) {
  if (on) {
    turnOn();
  } else {
    turnOff();
  }
}


float Incandescent::decayOn(unsigned long time) const {
  float x = (float)time / INCANDESCENT_TRANSITION_TIME;
  float y = (1-pow(0.01, x))*1.01; // approximate decay
  return y;
}

float Incandescent::decayOff(unsigned long time) const {
  float x = (float)time / INCANDESCENT_TRANSITION_TIME;
  float y = pow(0.01, x) * 1.01 - 0.01; // approximate decay
  return y;
}

// reverse the decayOn calculation
unsigned long Incandescent::reverseDecayOn(float y) const {
  float x = log(1-y/1.01) / log(0.01);
  return round(x * INCANDESCENT_TRANSITION_TIME);
}

// reverse the decayOff calculation
unsigned long Incandescent::reverseDecayOff(float y) const {
  float x = log((y + 0.01) / 1.01) / log(0.01);
  return round(x * INCANDESCENT_TRANSITION_TIME);
}
