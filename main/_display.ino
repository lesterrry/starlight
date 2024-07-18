class Display {
  public:
    enum Layout {
      List,
      ListWithBigTitle
    };

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

    void print(char character) {
      _display.print(character);
    }

    void render() {
      _display.display();
    }

    void clear(bool shouldRender = true) {
      _display.clearDisplay();

      if (shouldRender) render();
    }

    void setTextSize(uint8_t size) {
      _display.setTextSize(size);
    }

    void setTextColor(uint16_t color) {
      _display.setTextColor(color);
    }

    void setCursor(int16_t x, int8_t y) {
      _display.setCursor(x, y * 8);
    }

    void printCornerChar(char character) {
      setTextSize(1);
      setCursor(rightmostPixelSize1, 0);
      print(character);
    }

    void renderTitle(String title, String subtitle = "") {
      _display.clearDisplay();
      
      setTextSize(2);
      setCursor(0, 1);
      print(title);

      setTextSize(1);
      setCursor(0, 3);
      print(subtitle);
    }

    void printCursor(Layout forLayout = List, uint8_t position = 1) {
      if (forLayout == List) {
        setTextSize(1);
        setCursor(rightmostPixelSize1, position);
        print(cursorChar);
      } else {
        setTextSize(2);
        setCursor(rightmostPixelSize2, position);
        print(cursorChar);
      }
    }

    void renderLayout(Layout layout = List, bool clear = true, String upperNote = "", String title = "", String subtitle = "", String lowerNote = "") {
      if (clear) _display.clearDisplay();

      setTextSize(1);
      setCursor(0, 0);
      print(upperNote);

      if (layout == List) {
        setTextSize(1);
        setCursor(0, 1);
        print(title);

        setTextSize(1);
        setCursor(0, 2);
        print(subtitle);

        setTextSize(1);
        setCursor(0, 3);
        print(lowerNote);
      } else {
        setTextSize(2);
        setCursor(0, 1);
        print(title);

        setTextSize(1);
        setCursor(0, 3);
        print(lowerNote);
      }

      render();
    }

  private:
    Adafruit_SSD1306 _display;
    uint16_t rightmostPixelSize1 = 120;
    uint16_t rightmostPixelSize2 = 116;
    char cursorChar = '<';
};
