
#pragma once

#include "wifi.h"

typedef enum {
  STATE_DISCONNECTED,
  STATE_CONNECTING,
  STATE_CONNECTED,
} ConnState;

class WifiLedClass {
  uint8_t pin = 0xff;
  ConnState state = STATE_DISCONNECTED;
  unsigned long ledBlinkStart = 0;

  public:
    void begin(uint8_t pin) {
      this->pin = pin;
      pinMode(pin, OUTPUT);
      digitalWrite(pin, LOW);
    }

    void loop() {
      ConnState oldState = state;

      switch (WiFi.status()) {
        case WL_CONNECTED:
          state = STATE_CONNECTED;
          break;
        case WL_IDLE_STATUS:
          state = STATE_CONNECTING;
          break;
        case WL_DISCONNECTED:
          state = STATE_DISCONNECTED;
          break;
        default:
          // assume disconnected (TODO: other indicator)
          state = STATE_DISCONNECTED;
          break;
      }

      if (state != oldState) {
        switch (state) {
          case STATE_CONNECTING:
            // It looks like this doesn't actually work.
            ledBlinkStart = millis();
            Serial.println("Connecting...");
            break;
          case STATE_CONNECTED:
            Serial.print("Connected with IP address: ");
            Serial.println(WiFi.localIP());
            break;
          case STATE_DISCONNECTED:
            Serial.println("Disconnected.");
            break;
        }
      }

      digitalWrite(pin, ledHigh());
    }


    void busy() {
      digitalWrite(pin, !ledHigh());
    }

    void done() {
      digitalWrite(pin, ledHigh());
    }

  private:
    bool ledHigh() {
      switch (state) {
        case STATE_DISCONNECTED:
          return HIGH; // off
        case STATE_CONNECTING:
          return (millis() - ledBlinkStart) % 200 >= 100;
        case STATE_CONNECTED:
          return LOW; // on
      }
    }
};

WifiLedClass WifiLed;
