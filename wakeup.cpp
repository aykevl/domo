
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

void WakeupLight::setWakeup(int32_t dayTime, int32_t duration) {
  if (time.setDayTime(dayTime)) {
    Settings.data.wake_hour   = time.getHour();
    Settings.data.wake_minute = time.getMinute();
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

void WakeupLight::off() {
  _off();
  sendState();
}

void WakeupLight::_off() {
  float y = currentBrightness();
  state = LIGHT_SWITCH;
  nextState = LIGHT_OFF;
  transitionStart = millis() - (1.0-y)*LIGHT_TIME_FADE;
  loop();
}

void WakeupLight::wake() {
  _wake();
  sendState();
}

void WakeupLight::_wake() {
  float y = currentBrightness();
  state = LIGHT_SWITCH;
  nextState = LIGHT_WAKE;
  transitionStart = millis() - (1.0-y)*LIGHT_TIME_FADE;
  loop();
}

void WakeupLight::on() {
  _on();
  sendState();
}

void WakeupLight::_on() {
  float y = currentBrightness();
  state = LIGHT_SWITCH;
  nextState = LIGHT_ON;
  transitionStart = millis() - y*LIGHT_TIME_FADE;
  loop();
}

lightState_t WakeupLight::currentState() {
  if (state == LIGHT_SWITCH) {
    return nextState;
  } else {
    return state;
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
    case LIGHT_ON:
      return 1.0;
    case LIGHT_SWITCH: {
      // Formula for turn-on transition
      float y = (float)(millis() - transitionStart) / LIGHT_TIME_FADE;
      if (y >= 1.0) {
        state = nextState;
        return currentBrightness();
      }
      switch (nextState) {
        case LIGHT_OFF:
        case LIGHT_WAKE:
          return 1.0 - y;
        case LIGHT_ON:
          return y;
      }
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

void WakeupLight::sendState() {
  DynamicJsonBuffer jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["origin"] = CLIENT_ID;
  JsonObject& values = root.createNestedObject("value");

  switch (currentState()) {
    case LIGHT_OFF:
      values["state"] = "off";
      break;
    case LIGHT_WAKE:
      values["state"] = "wake";
      break;
    case LIGHT_ON:
      values["state"] = "on";
      break;
  }
  values["time"] = time.dayTime();
  values["duration"] = duration / 1000.0;
  values["switchDuration"] = LIGHT_TIME_FADE / 1000.0;

  const size_t messageMaxLen = 128;
  uint8_t message[messageMaxLen];
  size_t messageLen = root.printTo((char*)message, messageMaxLen);

  mqtt.publish(MQTT_PREFIX "a/wakeup", message, messageLen, true);
}

void WakeupLight::recvState(JsonObject &value) {
  log("got wakeup change");

  const char *state = value["state"];
  if (state != NULL) {
    lightState_t newState = LIGHT_UNDEFINED;
    if (strcmp(state, "off") == 0) {
      newState = LIGHT_OFF;
    } else if (strcmp(state, "wake") == 0) {
      newState = LIGHT_WAKE;
    } else if (strcmp(state, "on") == 0) {
      newState = LIGHT_ON;
    }
    if (newState != LIGHT_UNDEFINED && newState != currentState()) {
      switch (newState) {
        case LIGHT_OFF:
          _off();
          break;
        case LIGHT_WAKE:
          _wake();
          break;
        case LIGHT_ON:
          _on();
          break;
      }
    }
  }

  uint32_t newDuration = (uint32_t)value["duration"] / 60;
  uint32_t newTime = value["time"];

  setWakeup(newTime, newDuration);
}
