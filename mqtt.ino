
#include <Adafruit_MQTT.h>
#include <Adafruit_MQTT_Client.h>

void messageReceived(String topic, String payload, char * bytes, unsigned int length) {
  // Do nothing with it for now.
}

#ifdef MQTT_LOG
void mqttLog(String line) {
  line = String(F(CLIENT_ID ": ")) + line;
  mqtt.publish(MQTT_LOG, (uint8_t*)line.c_str(), line.length(), 1);
}
#else
#define mqttLog(line)
#endif
