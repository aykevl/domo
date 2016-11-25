
#pragma once

enum {
  RADIO_MSG_NONE,
  RADIO_MSG_COLOR,
};

bool radioSend(uint8_t messageType, uint8_t *data, size_t length);
