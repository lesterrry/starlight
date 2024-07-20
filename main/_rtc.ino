class RTC {
  public:
    bool isConnected() {
      return _rtc.begin();
    }

    void setTime(uint8_t seconds, uint8_t minutes, uint8_t hours, uint8_t day, uint8_t month, uint16_t year) {
      _rtc.setTime(seconds, minutes, hours, day, month, year);
    }

    void setCompileTime() {
      setTime(BUILD_SEC, BUILD_MIN, BUILD_HOUR, BUILD_DAY, BUILD_MONTH, BUILD_YEAR);
    }

    float getTemp() {
      return _rtc.getTemperatureFloat();
    }

    DateTime now() {
      return _rtc.getTime();
    }

    uint32_t unix() {
      return _rtc.getUnix(TIMEZONE);
    }

    uint16_t minutesSinceMidnight() {
      return _rtc.getHours() * 60 + _rtc.getMinutes();
    }

    String getTimeFromMsm(uint16_t msm) {
      return _formatTime(msm / 60, msm % 60, 0, false);
    }

    String getTime(bool withSeconds = false) {
      return _formatTime(_rtc.getHours(), _rtc.getMinutes(), _rtc.getSeconds(), withSeconds);
    }

    String getDate() {
      String date = "";

      int8_t day = _rtc.getDate();
      if (day < 10) date += '0';
      date += String(day);

      date += '.';

      int8_t month = _rtc.getMonth();
      if (month < 10) date += '0';
      date += String(month);

      return date;
    }

    String getUnixDelta(uint32_t pastTime) {
      uint32_t current = unix();
      uint32_t delta = current - pastTime;

      return unixToString(delta);
    }

    String unixToString(long seconds) {
      String timeString;

      int days = seconds / 86400;
      seconds = seconds % 86400;
      
      int hours = seconds / 3600;
      seconds = seconds % 3600;
      
      int minutes = seconds / 60;
      seconds = seconds % 60;

      if (days > 0) {
        timeString += String(days) + "d ";
      }
      if (hours > 0) {
        timeString += String(hours) + "h ";
      }
      if (minutes > 0) {
        timeString += String(minutes) + "m ";
      }
      timeString += String(seconds) + "s";

      return timeString;
    }

  private:
    MicroDS3231 _rtc;

    String _formatTime(uint8_t hour, uint8_t minute, uint8_t second, bool withSeconds) {
      String time = "";

      if (hour < 10) time += '0';
      time += String(hour);

      time += ':';

      if (minute < 10) time += '0';
      time += String(minute);

      if (withSeconds) {
        time += ':';

        if (second < 10) time += '0';
        time += String(second);
      }

      return time;
    }
};
