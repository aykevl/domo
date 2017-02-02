
#include "settings.h"
#include "light.h"
#include "button.h"
#include "htsensor.h"

Light light1;
Light light2;
Button button = Button(2);

void setup() {
  Serial.begin(9600); // Higher bitrates appear unreliable on the fake Chinese Nano
  Serial.println("lights start");
  Settings.begin();
  light1.begin(5, 1, &Settings.data.light1);
  light2.begin(6, 2, &Settings.data.light2);
  radioSetup();
  htsensorSetup();
}

void loop() {
  radioLoop();
  light1.loop();
  light2.loop();
  button.loop();
  htsensorLoop();

  static bool buttonWasPressed = false;
  bool buttonPressed = button.pressed();
  if (buttonPressed && !buttonWasPressed) {
    Serial.println("button press!");
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
