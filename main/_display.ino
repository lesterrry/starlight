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

    void setCursor(int16_t x, int8_t y) {
      _display.setCursor(x, y * 8);
    }

    void renderTitle(String title, String subtitle = "") {
      _display.clearDisplay();
      
      setTextSize(2);
      setCursor(0, 1);
      print(title);

      setTextSize(1);
      setCursor(0, 3);
      print(subtitle);

      render();
    }

    void renderLayout(Layout layout = List, String upperNote = "", String title = "", String subtitle = "", String lowerNote = "") {
      _display.clearDisplay();

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
};
