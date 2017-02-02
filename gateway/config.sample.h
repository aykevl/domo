#include "secrets.h"

// Unique ID within the LAN and on the MQTT broker.
#define CLIENT_ID "gateway"

#define TIME_URL "https://aykevl.nl/api/time"
#define TIME_FINGERPRINT "0D:CA:9C:A0:41:B4:8E:5F:F1:C5:62:48:4B:A0:BC:61:9A:B5:5F:FE"

// MQTT connection config.
#define MQTT_HOST "aykevl.nl"
#define MQTT_PORT 1883
#define MQTT_USER "domo"
#define MQTT_PASS LOGIN_PASSWORD
#define MQTT_PREFIX "me/home/"
#define MQTT_LOG "me/log/" CLIENT_ID

// 32 random bits to use as the start address of the RF24.
#define RF24_ADDRESS 0x00000000
#define RF24_CHANNEL 71

#define SERIAL_ENABLED 1
