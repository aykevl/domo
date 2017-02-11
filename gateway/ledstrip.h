
#pragma once

typedef enum {
  LEDSTRIP_OFF,
  LEDSTRIP_COLOR,
} ledstripMode_t;

void ledstripSend(uint8_t *arg);
void ledstripReceive(JsonObject &value);
