
#include <Arduino.h>
#include <ArduinoJson.h>

#include "wakeup.h"
#include "settings.h"
#include "mqtt.h"
#include "config.h"

WakeupLight wakeup;

void WakeupLight::begin(uint8_t pin) {
  this->pin = pin;
  this->state = LIGHT_OFF;
  this->transitionStart = millis();
  this->wasInWakeup = false;
  time.setHour(Settings.data.wake_hour);
  time.setMinute(Settings.data.wake_minute);
  duration = Settings.data.wake_duration * 60000;

  pinMode(pin, OUTPUT);
  digitalWrite(pin, 0);
}

void WakeupLight::setWakeup(int32_t hour, int32_t minute, int32_t duration) {
  if (time.setHour(hour)) {
    Settings.data.wake_hour = hour;
  }

  if (time.setMinute(minute)) {
    Settings.data.wake_minute = minute;
  }

  if (duration >= 0 && duration <= 60) {
    this->duration = duration * 60000;
    Settings.data.wake_duration = duration;
  }

  Settings.save();
}

void WakeupLight::loop() {
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

void WakeupLight::wake() {
  // We could also start from the current brightness:
  //float y = currentBrightness();
  //float x = log((y * 255.0) + 1.0) / log(2) / 8.0; // inverse of LIGHT_WAKE case in currentBrightness
  state = LIGHT_WAKE;
  transitionStart = millis(); // - x*duration;
  loop();
}
void WakeupLight::on() {
  float y = currentBrightness();
  state = LIGHT_FASTSTART;
  transitionStart = millis() - y*LIGHT_TIME_FADE;
  loop();
}

void WakeupLight::off() {
  float y = currentBrightness();
  state = LIGHT_STOP;
  transitionStart = millis() - (1.0-y)*LIGHT_TIME_FADE;
  loop();
}

lightState_t WakeupLight::currentState() {
  switch (state) {
    case LIGHT_WAKE:
      return LIGHT_WAKE;
    case LIGHT_FASTSTART:
    case LIGHT_ON:
      return LIGHT_ON;
    case LIGHT_STOP:
    case LIGHT_OFF:
      return LIGHT_OFF;
  }
}

float WakeupLight::currentBrightness() {
  switch (state) {
    case LIGHT_OFF:
      return 0.0;
    case LIGHT_WAKE: {
      float x = (float)(millis() - transitionStart) / duration;
      float y = (pow(2.0, x*8.0) - 1.0) / 255.0;
      if (y > 1.0) {
        state = LIGHT_ON;
        return 1.0;
      }
      return y;
    }
    case LIGHT_FASTSTART: {
      float y = (float)(millis() - transitionStart) / LIGHT_TIME_FADE;
      if (y > 1.0) {
        state = LIGHT_ON;
        return 1.0;
      }
      return y;
    }
    case LIGHT_ON:
      return 1.0;
    case LIGHT_STOP: {
      float y = 1.0 - (float)(millis() - transitionStart) / LIGHT_TIME_FADE;
      if (y < 0.0) {
        state = LIGHT_OFF;
        return 0.0;
      }
      return y;
    }
  }
}
bool WakeupLight::inWakeup() {
  uint32_t now = Clock.timestamp();
  if (now == 0) {
    // Clock is not yet initialized.
    return false;
  }
  uint32_t ts_now = Time(now).dayTime(); // seconds in this day (including TZ)
  uint32_t ts_wake = time.dayTime();
  return ts_now >= ts_wake && ts_now <= ts_wake+(duration/1000);
}
