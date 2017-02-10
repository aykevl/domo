
#pragma once

#include <ArduinoJson.h>

void amplifierSetup();
void amplifierLoop();
void amplifierSendState();
void amplifierRecvState(JsonObject &value);
