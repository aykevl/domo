
#pragma once

#include <ESP8266WiFi.h>

#include "time.h"
#include "wifi.h"
#include "config.h"

Time::Time(int16_t timezone) {
  this->hour     = 0;
  this->minute   = 0;
  this->second   = 0;
  this->timezone = timezone;
}

Time::Time(uint32_t unixSeconds, int16_t timezone) {
  uint32_t s = (unixSeconds+(int32_t)timezone*60) % 86400;
  this->hour     = s / 3600;
  this->minute   = s / 60 % 60;
  this->second   = s % 60;
  this->timezone = timezone;
}

Time::Time(uint8_t hour, uint8_t minute, uint8_t second, int16_t timezone) {
  this->hour     = hour;
  this->minute   = minute;
  this->second   = second;
  this->timezone = timezone;
}

bool Time::setHour(int32_t hour) {
  if (hour < 0 || hour > 23) {
    return false;
  }
  this->hour = hour;
  return true;
}

bool Time::setMinute(int32_t minute) {
  if (minute < 0 || minute > 59) {
    return false;
  }
  this->minute = minute;
  return true;
}

bool Time::setSecond(int32_t second) {
  if (second < 0 || second > 59) {
    return false;
  }
  this->second = second;
  return true;
}

int32_t Time::dayTime() {
  return ((int32_t)hour * 3600 + (int32_t)minute * 60 + (int32_t)second) - (int32_t)timezone * 60;
}

bool Time::setDayTime(int32_t dayTime) {
  dayTime += (int32_t)timezone * 60;
  if (dayTime < 0 || dayTime > 86400) {
    return false;
  }

  this->hour     = dayTime / 3600;
  this->minute   = dayTime / 60 % 60;
  this->second   = dayTime % 60;
  return true;
}

String Time::format() {
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

String Time::formatHour() {
  String s("00");
  String hour_str = String(hour);
  putstr(s, 0, hour_str);
  return s;
}

String Time::formatMinute() {
  String s("00");
  String minute_str = String(minute);
  putstr(s, 0, minute_str);
  return s;
}

void Time::putstr(String &s, uint8_t start, String &add) {
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

Date::Date(uint32_t unixSeconds, int16_t timezone) {
  this->timezone = timezone;
  this->day = (unixSeconds+(int32_t)timezone*60) / 86400;
}

uint32_t Date::timestamp() {
  return day * 86400 - ((int32_t)timezone*60);
}

uint32_t Date::timestamp(Time time) {
  if (timezone != time.getTimezone()) {
    return 0; // TODO?
  }
  return timestamp() + time.dayTime() // add day and time
    + (int32_t)timezone*60;           // undo double timezone
}


void ClockClass::loop() {
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

uint64_t ClockClass::timestamp_ms() {
  if (millisAtUnixTime == 0) {
    return 0;
  }
  return lastUnixMillis + (uint64_t)(millis() - millisAtUnixTime);
}

// uint32_t Unix time will only roll over in the year 2109, so should
// suffice.
uint32_t ClockClass::timestamp() {
  return timestamp_ms() / 1000;
}

Time ClockClass::UTCTime() {
  return Time(timestamp());
}

uint32_t ClockClass::nextTimestamp(Time nextTime) {
  uint32_t now = timestamp();
  Date date(now);
  Time t(now);

  if (date.timestamp(nextTime) < now) {
    date.addDay(1);
  }

  return date.timestamp(nextTime);
}

ClockClass Clock;
