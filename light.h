
#pragma once

#include <Arduino.h>

typedef enum {
  LIGHT_OFF,
  LIGHT_SLOWSTART, // in minutes
  LIGHT_FASTSTART, // in seconds
  LIGHT_ON,
  LIGHT_STOP,      // in seconds
} lightState_t;

// Transition times in ms
const float LIGHT_TIME_SLOWSTART = 10000;
const float LIGHT_TIME_FASTSTART = 1000;
const float LIGHT_TIME_STOP      = 1000;

class WakeupLight {
  public:

    WakeupLight(uint8_t pin) {
      this->pin = pin;
      this->state = LIGHT_OFF;
      this->transitionStart = millis();

      pinMode(pin, OUTPUT);
    }

    void loop() {
      analogWrite(pin, currentBrightness()*255);
    }

    void slowStart() {
      float y = currentBrightness();
      float x = log((y * 255.0) + 1.0) / log(2) / 8.0; // inverse of LIGHT_SLOWSTART case in currentBrightness
      state = LIGHT_SLOWSTART;
      transitionStart = millis() - x*LIGHT_TIME_SLOWSTART;
      loop();
    }

    void fastStart() {
      float y = currentBrightness();
      state = LIGHT_FASTSTART;
      transitionStart = millis() - y*LIGHT_TIME_FASTSTART;
      loop();
    }

    void stop() {
      float y = currentBrightness();
      state = LIGHT_STOP;
      transitionStart = millis() - (1.0-y)*LIGHT_TIME_STOP;
      loop();
    }

  private:
    float currentBrightness() {
      switch (state) {
        case LIGHT_OFF:
          return 0.0;
        case LIGHT_SLOWSTART: {
          float x = (float)(millis() - transitionStart) / LIGHT_TIME_SLOWSTART;
          float y = (pow(2.0, x*8.0) - 1.0) / 255.0;
          if (y > 1.0) {
            state = LIGHT_ON;
            return 1.0;
          }
          return y;
        }
        case LIGHT_FASTSTART: {
          float y = (float)(millis() - transitionStart) / LIGHT_TIME_FASTSTART;
          if (y > 1.0) {
            state = LIGHT_ON;
            return 1.0;
          }
          return y;
        }
        case LIGHT_ON:
          return 1.0;
        case LIGHT_STOP: {
          float y = 1.0 - (float)(millis() - transitionStart) / LIGHT_TIME_STOP;
          if (y < 0.0) {
            state = LIGHT_OFF;
            return 0.0;
          }
          return y;
        }
      }
    }

    uint8_t pin;
    lightState_t state;
    unsigned long transitionStart;
};
