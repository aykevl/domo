
#include "light.h"
#include "settings.h"
#include "radio.h"

void Light::begin(uint8_t pin, uint8_t child, SettingsDataLight *settings) {
  this->pin = pin;
  this->child = child;
  this->state = LIGHT_OFF;
  this->transitionStart = millis();
  this->wasInWakeup = false;
  this->settings = settings;
  time.setHour(settings->hour);
  time.setMinute(settings->minute);
  duration = settings->duration * 60000;
  enabled = settings->enabled;
  fullBrightness = settings->fullBrightness;

  pinMode(pin, OUTPUT);
  digitalWrite(pin, HIGH);
}

void Light::setWakeup(int32_t hour, int32_t minute, int32_t duration) {
  if (time.setHour(hour)) {
    settings->hour = hour;
  }

  if (time.setMinute(minute)) {
    settings->minute = minute;
  }

  if (duration >= 0 && duration <= 60) {
    this->duration = duration * 60000;
    settings->duration = duration;
  }

  Settings.save();
}

void Light::loop() {
  if (enabled) {
    bool nowInWakeup = inWakeup();
    if (!wasInWakeup && nowInWakeup) {
      // transition into wakeup
      if (state == LIGHT_OFF) {
        wake();
      }
    }
    wasInWakeup = nowInWakeup;
  }

  // TODO: make fullBrightness logarithmic.
  float brightness = currentBrightness() * fullBrightness;
  if (brightness >= 1.0) {
    digitalWrite(pin, HIGH);
  } else if (brightness <= 0.0) {
    digitalWrite(pin, LOW);
  } else {
    analogWrite(pin, int(ceil(brightness*255.0)));
  }
}

void Light::setState(lightState_t newState) {
  if (currentState() == newState) {
    return;
  }
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
    default:
      // ignore unknown value
      break;
  }
}

void Light::off() {
  _off();
  sendState();
}

void Light::_off() {
  float y = currentBrightness();
  state = LIGHT_SWITCH;
  nextState = LIGHT_OFF;
  transitionStart = millis() - (1.0-y)*LIGHT_TIME_FADE;
  loop();
}

void Light::wake() {
  _wake();
  sendState();
}

void Light::_wake() {
  float y = currentBrightness();
  state = LIGHT_SWITCH;
  nextState = LIGHT_WAKE;
  transitionStart = millis() - (1.0-y)*LIGHT_TIME_FADE;
  loop();
}

void Light::on() {
  _on();
  sendState();
}

void Light::_on() {
  float y = currentBrightness();
  state = LIGHT_SWITCH;
  nextState = LIGHT_ON;
  transitionStart = millis() - y*LIGHT_TIME_FADE;
  loop();
}

lightState_t Light::currentState() {
  if (state == LIGHT_SWITCH) {
    return nextState;
  } else {
    return state;
  }
}

float Light::currentBrightness() {
  switch (state) {
    case LIGHT_OFF:
      return 0.0;
    case LIGHT_WAKE: {
      float x = (float)(millis() - transitionStart) / duration;
      float y = (pow(2.0, x*8.0) - 1.0) / 255.0;
      if (y > 1.0) {
        state = LIGHT_ON;
        sendState();
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

bool Light::inWakeup() {
  uint32_t now = Clock.timestamp();
  if (now == 0) {
    // Clock is not yet initialized.
    return false;
  }
  uint32_t ts_now = Time(now).dayTime();
  uint32_t ts_wake = time.dayTime();
  static uint32_t lastCheck = 0;
  if (lastCheck == 0 || lastCheck + 60 < now) {
      lastCheck = now;
  }
  return (ts_wake - ts_now + 86400) % 86400 < (duration/1000);
}

void Light::sendState() {
  uint8_t msg[12];
  memset(msg, 0, sizeof(msg));
  msg[0] = RADIO_MSG_LIGHT;
  msg[1] = child;
  uint8_t *arg = msg+2;
  arg[0] = currentState();
  if (enabled) {
    arg[0] |= LIGHT_FLAG_ENABLED;
  }
  arg[1] = fullBrightness * 255 + 0.5;
  uint32_t timestamp = Clock.timestamp();
  for (uint8_t i=0; i<4; i++) { // not very useful to send the time...
    arg[2+i] = uint8_t(timestamp);
    timestamp >>= 8;
  }
  arg[6] = time.getHour();
  arg[7] = time.getMinute();
  arg[8] = duration / 256;
  arg[9] = duration * 256;

  if (!radioSend(msg, sizeof(msg))) {
    Serial.println(F("failed to send light message"));
  }
}

void Light::gotMessage(uint8_t *arg) {
  setState(lightState_t(arg[0] & LIGHT_FLAG_STATUS_MASK));
  enabled = arg[0] & LIGHT_FLAG_ENABLED != 0;
  fullBrightness = float(arg[1]) / 255.0;
  uint32_t timestamp = 0;
  for (uint8_t i=4; i; i--) { // handle 4 bytes (in little-endian format)
    timestamp <<= 8;
    timestamp |= arg[2+(i-1)];
  }
  Clock.setTime(timestamp); // updated time
  int32_t duration = int32_t(arg[8]) * 256 + arg[9];
  setWakeup(arg[6], arg[7], duration); // hour, minute, duration
}
