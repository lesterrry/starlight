class D2D {
  public:
    D2D(float lon, float lat, uint8_t tz) {
      _provider = new Dusk2Dawn(lon, lat, tz);
    };

    SolarTime getSolarTime(uint8_t day, uint8_t month, uint16_t year, int8_t duskOffset = 0, int8_t dawnOffset = 0) {
      SolarTime time;
      time.sunsetMsm = _provider->sunset(year, month, day, false) + duskOffset;
      time.sunriseMsm = _provider->sunrise(year, month, day, false) + dawnOffset;

      return time;
    };

  private:
    Dusk2Dawn* _provider;
};
