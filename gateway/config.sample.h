#include "secrets.h"

// Unique ID within the LAN and on the MQTT broker.
#define CLIENT_ID "gateway"

#define TIME_URL "http://notls.aykevl.nl/api/time"
//#define TIME_FINGERPRINT "00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00" // fill in the certificate fingerprint

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

// When serial is enabled, the rotary button will be disabled (they use the same pin).
//#define SERIAL_ENABLED 1

// It looks like the ESP crashes as soon as the radio is enabled when it isn't available.
#define RADIO_ENABLED 1
