
#pragma once

enum {
  RADIO_MSG_NONE,
  RADIO_MSG_TIME,
  RADIO_MSG_COLOR,
  RADIO_MSG_LIGHT,
  RADIO_MSG_HT,
};

const uint8_t RADIO_MSG_REQUEST = 0x80;

bool radioSend(uint8_t *msg, size_t length);
