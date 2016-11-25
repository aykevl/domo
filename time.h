
#pragma once

#include <ESP8266HTTPClient.h>

const int16_t TIMEZONE = 60; // +01:00

class Time {
  int16_t timezone;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;

public:
  Time(int16_t timezone=TIMEZONE);
  Time(uint32_t unixSeconds, int16_t timezone=TIMEZONE);
  Time(uint8_t hour, uint8_t minute, uint8_t second, int16_t timezone=TIMEZONE);
  uint8_t getHour() { return hour; }
  bool setHour(int32_t hour);
  uint8_t getMinute() { return minute; }
  bool setMinute(int32_t minute);
  uint8_t getSecond() { return second; }
  bool setSecond(int32_t second);
  uint8_t getTZHour() { return timezone / 60; }
  uint8_t getTZMinute() { return timezone % 60; }
  uint16_t getTimezone() { return timezone; }
  int32_t dayTime();
  String format();
  String formatHour();
  String formatMinute();

private:
  void putstr(String &s, uint8_t start, String &add);
};


class Date {
  uint32_t day;
  int16_t timezone;

public:
  Date(uint32_t unixSeconds, int16_t timezone=TIMEZONE);
  void addDay(uint32_t day) { this->day += day; }
  uint32_t timestamp();
  uint32_t timestamp(Time time);
};


class ClockClass {
  const uint32_t CHECK_TIMEOUT = 10000;

  uint64_t lastUnixMillis = 0;
  uint32_t millisAtUnixTime = 0;
  uint32_t lastCheck = 0;
  HTTPClient http;

public:
  void loop();
  uint64_t timestamp_ms();
  uint32_t timestamp();
  Time UTCTime();
  uint32_t nextTimestamp(Time nextTime);
};

extern ClockClass Clock;
