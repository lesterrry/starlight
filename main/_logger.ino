class Logger {
  public:
    Logger(bool active, unsigned long baud = 9600) {
      _active = active;
      _baud = baud;
    }

    void begin() {
      if (_active) Serial.begin(_baud);
    }

    template <typename Generic>
    void print(Generic message) {
      if (_active) Serial.println(message);
    }

  private:
    int _active;
    unsigned long _baud;
};
