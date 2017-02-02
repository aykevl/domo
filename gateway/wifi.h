
#pragma once

class WiFiClass {
  bool mdnsStarted = false;

  public:
    void setup();
    void loop();
};

extern WiFiClass wifi;
