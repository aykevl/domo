
#pragma once

#include "wifi.h"

const int16_t TIMEZONE = 60; // +01:00

class Time {
  int16_t timezone;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;

public:
  Time(int16_t timezone=TIMEZONE) {
    this->hour     = 0;
    this->minute   = 0;
    this->second   = 0;
    this->timezone = timezone;
  }

  Time(uint32_t unixSeconds, int16_t timezone=TIMEZONE) {
    uint32_t s = (unixSeconds+(int32_t)timezone*60) % 86400;
    this->hour     = s / 3600;
    this->minute   = s / 60 % 60;
    this->second   = s % 60;
    this->timezone = timezone;
  }

  Time(uint8_t hour, uint8_t minute, uint8_t second, int16_t timezone=TIMEZONE) {
    this->hour     = hour;
    this->minute   = minute;
    this->second   = second;
    this->timezone = timezone;
  }

  uint8_t getHour() {
    return hour;
  }

  bool setHour(int32_t hour) {
    if (hour < 0 || hour > 23) {
      return false;
    }
    this->hour = hour;
    return true;
  }

  uint8_t getMinute() {
    return minute;
  }

  bool setMinute(int32_t minute) {
    if (minute < 0 || minute > 59) {
      return false;
    }
    this->minute = minute;
    return true;
  }

  uint8_t getSecond() {
    return second;
  }

  bool setSecond(int32_t second) {
    if (second < 0 || second > 59) {
      return false;
    }
    this->second = second;
    return true;
  }

  uint8_t getTZHour() {
    return timezone / 60;
  }

  uint8_t getTZMinute() {
    return timezone % 60;
  }

  // Get the timezone offset from UTC, in minutes
  uint16_t getTimezone() {
    return timezone;
  }

  int32_t dayTime() {
    return ((int32_t)hour * 3600 + (int32_t)minute * 60 + (int32_t)second) - (int32_t)timezone * 60;
  }

  String format() {
    // TODO: negative time zone
    String hours = String(hour);
    String minutes = String(minute);
    String seconds = String(second);
    String tzhour  = String(getTZHour());
    String tzmin   = String(getTZMinute());
    String s("00:00:00+00:00");
    putstr(s, 0, hours);
    putstr(s, 3, minutes);
    putstr(s, 6, seconds);
    putstr(s, 9, tzhour);
    putstr(s, 12, tzmin);
    return s;
  }

  String formatHour() {
    String s("00");
    String hour_str = String(hour);
    putstr(s, 0, hour_str);
    return s;
  }

  String formatMinute() {
    String s("00");
    String minute_str = String(minute);
    putstr(s, 0, minute_str);
    return s;
  }

private:
  void putstr(String &s, uint8_t start, String &add) {
    if (add.length() > 2) {
      // unsafe to do any work in this condition
      return;
    }
    if (add.length() == 2) {
      s[start+0] = add[0];
      s[start+1] = add[1];
    } else if (add.length() == 1) {
      s[start+1] = add[0];
    }
  }
};

class Date {
  uint32_t day;
  int16_t timezone;

  public:
  Date(uint32_t unixSeconds, int16_t timezone=TIMEZONE) {
    this->timezone = timezone;
    this->day = (unixSeconds+(int32_t)timezone*60) / 86400;
  }

  void addDay(uint32_t day) {
    this->day += day;
  }

  uint32_t timestamp() {
    return day * 86400 - ((int32_t)timezone*60);
  }

  uint32_t timestamp(Time time) {
    if (timezone != time.getTimezone()) {
      return 0; // TODO?
    }
    return timestamp() + time.dayTime() // add day and time
      + (int32_t)timezone*60;           // undo double timezone
  }
};


class ClockClass {
  const uint32_t CHECK_TIMEOUT = 10000;

  uint64_t lastUnixMillis = 0;
  uint32_t millisAtUnixTime = 0;
  uint32_t lastCheck = 0;
  HTTPClient http;

public:
  void loop() {
    if ((lastCheck == 0 || millis() - lastCheck > CHECK_TIMEOUT) && lastUnixMillis == 0 && WiFi.status() == WL_CONNECTED) {
      // no time is known yet
      lastCheck = millis();

      // This blocks
      Serial.println("Clock: requesting time...");
      http.begin(TIME_URL, TIME_FINGERPRINT);

      int httpCode = http.GET();
      if (httpCode == 200) {
        String payload = http.getString();
        // TODO: parse as 64-bit integer, and parse fractional part
        lastUnixMillis = (uint64_t)payload.toInt() * 1000;
        millisAtUnixTime = millis();
        Serial.print("Clock: timestamp=");
        Serial.println((uint32_t)timestamp());
      } else if (httpCode < 0) {
        Serial.printf("Clock: GET failed: %s\n", http.errorToString(httpCode).c_str());
      } else {
        Serial.printf("Clock: GET unknown HTTP code: %d\n", httpCode);
      }
    }
  }

  uint64_t timestamp_ms() {
    if (millisAtUnixTime == 0) {
      return 0;
    }
    return lastUnixMillis + (uint64_t)(millis() - millisAtUnixTime);
  }

  // uint32_t Unix time will only roll over in the year 2109, so should
  // suffice.
  uint32_t timestamp() {
    return timestamp_ms() / 1000;
  }

  Time UTCTime() {
    return Time(timestamp());
  }

  uint32_t nextTimestamp(Time nextTime) {
    uint32_t now = timestamp();
    Date date(now);
    Time t(now);

    if (date.timestamp(nextTime) < now) {
      date.addDay(1);
    }

    return date.timestamp(nextTime);
  }
};

ClockClass Clock;
