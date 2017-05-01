
#include "settings.h"
#include "config.h"
#include "light.h"
#include "button.h"
#include "htsensor.h"
#include "ledstrip.h"

Light light1;
Light light2;
Button button(2);
Ledstrip ledstrip(4, Button(3));

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
  button.loop();
  htsensorLoop();
  ledstrip.loop();

  static bool buttonWasPressed = false;
  bool buttonPressed = button.pressed();
  if (buttonPressed && !buttonWasPressed) {
#ifdef USE_SERIAL
    Serial.println("button press!");
#endif
    switch (light1.currentState()) {
      case LIGHT_OFF:
      case LIGHT_WAKE:
        light1.on();
        break;
      case LIGHT_ON:
        light1.off();
        break;
    }
  }
  buttonWasPressed = buttonPressed;
}
