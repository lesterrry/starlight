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

    uint8_t getSeconds() {
      return _rtc.getSeconds();
    }

    uint8_t getMinutes() {
      return _rtc.getMinutes();
    }

    uint8_t getHours() {
      return _rtc.getHours();
    }
    
    uint8_t getDay() {
      return _rtc.getDate();
    }

    uint8_t getMonth() {
      return _rtc.getMonth();
    }

    uint16_t getYear() {
      return _rtc.getYear();
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
      return _formatTime(getHours(), getMinutes(), getSeconds(), withSeconds);
    }

    String getDate(bool withYear = false) {
      return _formatDate(getDay(), getMonth(), getYear(), withYear);
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

    String msmToString(uint16_t minutes) {
      String timeString;

      int hours = minutes / 60;
      minutes = minutes % 60;

      if (hours > 0) {
        timeString += String(hours) + "h ";
      }
      
      timeString += String(minutes) + "m ";

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

    String _formatDate(uint8_t day, uint8_t month, uint16_t year, bool withYear) {
      String date = "";

      if (day < 10) date += '0';
      date += String(day);

      date += '.';

      if (month < 10) date += '0';
      date += String(month);

      if (withYear) {
        date += '.';
        
        date += String(year);
      }

      return date;
    }
};
