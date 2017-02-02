
#pragma once

#include <PubSubClient.h>
#include <WiFiClient.h>

void mqttCallback(char *topic, byte *payload, unsigned int length);
void mqttLoop();
void log(String line);

extern WiFiClient mqttClient;
extern PubSubClient mqtt;
