
#include "button.h"
#include "light.h"

#if defined(__AVR_ATtiny84__)
const uint8_t LIGHT_PIN = 7;
const uint8_t BUTTON_PIN = 1;

#elif defined(__AVR_ATmega328P__)
const uint8_t LIGHT_PIN = 3;
const uint8_t BUTTON_PIN = 2;

#else
#error "Unknown microprocessor"
#endif

Button button(BUTTON_PIN);
WakeupLight light(LIGHT_PIN);

void setup() {
  light.slowStart();
}

void loop() {
  static uint8_t lightState = 1;
  static bool buttonWasPressed = false;

  button.loop();
  light.loop();

  bool buttonPressed = button.pressed();
  if (buttonPressed && !buttonWasPressed) {
    switch (lightState) {
      case 0:
        // off -> slow on
        lightState = 1;
        light.slowStart();
        break;
      case 1:
        // slow on -> fast on
        lightState = 2;
        light.fastStart();
        break;
      case 2:
        // on -> off
        lightState = 0;
        light.stop();
        break;
    }
  }
  buttonWasPressed = buttonPressed;
}
