
#pragma once

#include "time.h"
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

void ClockClass::setTime(uint32_t newTime) {
  lastUnixSecs = newTime;
  millisAtUnixTime = millis();
}

// uint32_t Unix time will only roll over in the year 2109, so should
// suffice.
uint32_t ClockClass::timestamp() {
  if (lastUnixSecs == 0) {
    return 0;
  }
  return lastUnixSecs + (millis() - millisAtUnixTime) / 1000;
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
