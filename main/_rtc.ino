class RTC {
  public:
    bool isConnected() {
      return _rtc.begin();
    }

    void setTime(int8_t seconds, int8_t minutes, int8_t hours, int8_t day, int8_t month, int16_t year) {
      _rtc.setTime(seconds, minutes, hours, day, month, year);
    }

    void setCompileTime() {
      setTime(BUILD_SEC, BUILD_MIN, BUILD_HOUR, BUILD_DAY, BUILD_MONTH, BUILD_YEAR);
    }

    DateTime now() {
      return _rtc.getTime();
    }

    uint32_t unix() {
      return _rtc.getUnix(TIMEZONE);
    }

    String getTime() {
      String time = "";

      int8_t hour = _rtc.getHours();
      if (hour < 10) time += '0';
      time += String(hour);

      time += ':';

      int8_t minute = _rtc.getMinutes();
      if (minute < 10) time += '0';
      time += String(minute);

      return time;
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

      if(days > 0){
        timeString += String(days) + "d ";
      }
      if(hours > 0){
        timeString += String(hours) + "h ";
      }
      if(minutes > 0){
        timeString += String(minutes) + "m ";
      }
      timeString += String(seconds) + "s";

      return timeString;
    }

  private:
    MicroDS3231 _rtc;
};
