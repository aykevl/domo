
#include <Arduino.h>

// Source:
// https://www.circuitsathome.com/mcu/programming/reading-rotary-encoder-on-arduino

const uint8_t ROTARY_PIN_A = 1; // TX
const uint8_t ROTARY_PIN_B = 3; // RX

int8_t rotaryValue = 0;

void rotaryInterrupt() {
  static int8_t enc_states[] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};
  static uint8_t old_AB = 0b00000011; // 'rest' is HIGH (as it is pulled high)

  old_AB <<= 2;                                  // remember previous state
  old_AB |= (digitalRead(ROTARY_PIN_A) << 1) |   // add current state (2 bits)
             digitalRead(ROTARY_PIN_B);
  rotaryValue += enc_states[old_AB & 0x0f];
}

void rotarySetup() {
  pinMode(ROTARY_PIN_A, INPUT_PULLUP);
  pinMode(ROTARY_PIN_B, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ROTARY_PIN_A), rotaryInterrupt, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ROTARY_PIN_B), rotaryInterrupt, CHANGE);
}

int8_t rotaryRead() {
  int8_t value = rotaryValue;
  rotaryValue = 0;
  return value;
}
