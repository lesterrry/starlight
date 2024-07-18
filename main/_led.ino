class LED {
  public:
    LED(uint16_t numLeds, uint8_t brightness) {
      _leds = new CRGB[numLeds];
      _numLeds = numLeds;
      _isOn = false;
      FastLED.addLeds<WS2811, LED_PIN, GRB>(_leds, numLeds);
      FastLED.setBrightness(brightness);
    }

    void on(CRGB color = CRGB::Orange) {
      fill_solid(_leds, _numLeds, color);
      FastLED.show();
      _isOn = true;
    }

    void off() {
      FastLED.clear();
      FastLED.show();
      _isOn = false;
    }

    void toggle() {
      _isOn = !_isOn;
      _isOn ? on() : off();
    }

  private:
    CRGB* _leds;
    uint16_t _numLeds;
    bool _isOn;
};

