
#pragma once

typedef enum {
  LIGHT_OFF,
  LIGHT_SLOWSTART, // in minutes
  LIGHT_FASTSTART, // in seconds
  LIGHT_ON,
  LIGHT_STOP,      // in seconds
} lightState_t;

// Transition times in ms
const float LIGHT_TIME_SLOWSTART = 1800000; // 30min
const float LIGHT_TIME_FASTSTART = 500;
const float LIGHT_TIME_STOP      = 500;

uint32_t wakeupDuration = LIGHT_TIME_SLOWSTART;
Time wakeupTime(7, 15, 0);

class WakeupLight {
  uint8_t pin;
  lightState_t state;
  unsigned long transitionStart;
  bool wasInWakeup;

  public:

    WakeupLight(uint8_t pin) {
      this->pin = pin;
      this->state = LIGHT_OFF;
      this->transitionStart = millis();
      this->wasInWakeup = false;

      pinMode(pin, OUTPUT);
      digitalWrite(pin, 0);
    }

    void loop() {
      bool nowInWakeup = inWakeup();
      if (!wasInWakeup && nowInWakeup) {
        // transition into wakeup
        if (state == LIGHT_OFF) {
          Serial.println("Light: Time to wake up!");
          wake();
        }
      }
      wasInWakeup = nowInWakeup;
      analogWrite(pin, ceil(currentBrightness()*1023.0));
    }

    void wake() {
      // We could also start from the current brightness:
      //float y = currentBrightness();
      //float x = log((y * 255.0) + 1.0) / log(2) / 8.0; // inverse of LIGHT_SLOWSTART case in currentBrightness
      state = LIGHT_SLOWSTART;
      transitionStart = millis(); // - x*wakeupDuration;
      loop();
    }

    void on() {
      float y = currentBrightness();
      state = LIGHT_FASTSTART;
      transitionStart = millis() - y*LIGHT_TIME_FASTSTART;
      loop();
    }

    void off() {
      float y = currentBrightness();
      state = LIGHT_STOP;
      transitionStart = millis() - (1.0-y)*LIGHT_TIME_STOP;
      loop();
    }

    lightState_t currentState() {
      switch (state) {
        case LIGHT_SLOWSTART:
          return LIGHT_SLOWSTART;
        case LIGHT_FASTSTART:
        case LIGHT_ON:
          return LIGHT_ON;
        case LIGHT_STOP:
        case LIGHT_OFF:
          return LIGHT_OFF;
      }
    }

    float currentBrightness() {
      switch (state) {
        case LIGHT_OFF:
          return 0.0;
        case LIGHT_SLOWSTART: {
          float x = (float)(millis() - transitionStart) / wakeupDuration;
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

  private:
    bool inWakeup() {
      Time now(Clock.timestamp());
      uint32_t ts_now = now.dayTime(); // seconds in this day (including TZ)
      uint32_t ts_wake = wakeupTime.dayTime();
      return ts_now >= ts_wake && ts_now <= ts_wake+(wakeupDuration/1000);
    }
};
