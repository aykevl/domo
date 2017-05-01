
#include "settings.h"
#include "config.h"
#include "light.h"
#include "htsensor.h"
#include "ledstrip.h"

Light light1;
Light light2;
Ledstrip ledstrip(2);

void setup() {
#ifdef USE_SERIAL
  Serial.begin(9600); // Higher bitrates appear unreliable on the fake Chinese Nano
  Serial.println("lights start");
#endif
  Settings.begin();
  light1.begin(5, 1, &Settings.data.light1);
  light2.begin(6, 2, &Settings.data.light2);
  ledstrip.begin();
  radioSetup();
  htsensorSetup();
}

void loop() {
  radioLoop();
  light1.loop();
  light2.loop();
  htsensorLoop();
  ledstrip.loop();
}
