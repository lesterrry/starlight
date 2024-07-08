class Radio {
  public:
    Radio(int pin) {  // constructor
      pinMode(pin, INPUT);
      _pin = pin;
    }

    bool isReceiving() {
      return digitalRead(_pin) == 1;
    }

  private:
    int _pin;
};
