class LED {
  public:
    LED(uint16_t numLeds, uint8_t brightness) {
      _leds = new CRGB[numLeds];
      _numLeds = numLeds;
      FastLED.addLeds<WS2811, LED_PIN, GRB>(_leds, numLeds);
      FastLED.setBrightness(brightness);
    }

    void light(CRGB color) {
      fill_solid(_leds, _numLeds, color);
      FastLED.show();
    }

  private:
    CRGB* _leds;
    uint16_t _numLeds;
};

