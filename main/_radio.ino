class Radio {
  public:
    Radio(int pin) {
      pinMode(pin, INPUT);
      _pin = pin;
    }

    bool isReceiving() {
      return digitalRead(_pin) == 1;
    }

  private:
    int _pin;
};
