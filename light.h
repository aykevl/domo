
#pragma once

#include <ArduinoJson.h>

#include "time.h"

typedef enum {
  LIGHT_UNDEFINED,
  LIGHT_OFF,
  LIGHT_WAKE,
  LIGHT_ON,
  LIGHT_SWITCH,
} lightState_t;

const uint8_t LIGHT_FLAG_ENABLED     = 0b10000000;
const uint8_t LIGHT_FLAG_STATUS_MASK = 0b00000011;

void lightSend(const char *child, uint8_t *arg);
void lightReceive(uint8_t child, JsonObject &value);
