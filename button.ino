
#include "button.h"

Button::Button() {
}

Button::Button(uint8_t pin) {
  this->pin = pin;
  pinMode(pin, INPUT_PULLUP);
}

// 0 if not pressed, 1 if pressed, 2 if long pressed
uint8_t Button::read() {
  // Trick:
  // Take every bounce state together with it's normal state, and return the
  // button state number.
  return state / 2;
}

bool Button::pressed() {
  return read() != RELEASED;
}

void Button::loop() {
  bool nowPressed = digitalRead(pin) == LOW;
  switch (state) {
    case RELEASED:
      if (nowPressed) {
        state = RELEASED_BOUNCE;
        time = millis();
      }
      break;
    case RELEASED_BOUNCE:
      if (!nowPressed) {
        // bounce detected
        state = RELEASED;
      } else if (millis() - time >= BOUNCE_TIMEOUT) {
        state = PRESSED;
        time = millis();
      }
      break;
    case PRESSED:
      if (!nowPressed) {
        state = PRESSED_BOUNCE;
        time = millis();
      } else if (millis() - time >= LONGPRESS_TIMEOUT) {
        state = LONGPRESSED;
      }
      break;
    case PRESSED_BOUNCE:
      if (nowPressed) {
        // bounce detected
        state = PRESSED;
        time = millis();
      } else if (millis() - time >= BOUNCE_TIMEOUT) {
        state = RELEASED;
      }
      break;
    case LONGPRESSED:
      if (!nowPressed) {
        state = LONGPRESSED_BOUNCE;
        time = millis();
      }
      break;
    case LONGPRESSED_BOUNCE:
      if (nowPressed) {
        state = LONGPRESSED;
      } else if (millis() - time >= BOUNCE_TIMEOUT) {
        state = RELEASED;
      }
      break;
  }
}
