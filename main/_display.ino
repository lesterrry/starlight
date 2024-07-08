class Display {
  public:
    Display(int mosiPin, int clkPin, int dcPin, int resetPin, int csPin) {
      _display = Adafruit_SSD1306(mosiPin, clkPin, dcPin, resetPin, csPin);
    }

    void begin() {
      _display.begin(SSD1306_SWITCHCAPVCC);
      setTextColor(WHITE);
      clear();
    }

    void print(String text) {
      _display.print(text);
    }

    void render() {
      _display.display();
    }

    void clear() {
      _display.clearDisplay();
      render();
    }

    void setTextSize(uint8_t size) {
      _display.setTextSize(size);
    }

    void setTextColor(uint16_t color) {
      _display.setTextColor(color);
    }

    void setCursor(int16_t x, int16_t y) {
      _display.setCursor(x, y);
    }

    void renderTitle(String title, String subtitle) {
      _display.clearDisplay();
      setTextSize(2);
      setCursor(0,8);
      print(title);
      setTextSize(1);
      setCursor(0,25);
      print(subtitle);
      render();
    }

  private:
    Adafruit_SSD1306 _display;
};
