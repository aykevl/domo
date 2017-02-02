
#pragma once

enum {
  RADIO_MSG_NONE,
  RADIO_MSG_COLOR,
  RADIO_MSG_LIGHT,
};

bool radioSend(uint8_t *msg, size_t length);
