class Knob {
  public:
    Knob(int clkPin, int dtPin, int swPin, bool type = TYPE2) {  // constructor
      _encoder = Encoder(clkPin, dtPin, swPin, type);
      _encoder.setType(type);
    }

    void update() {
      _encoder.tick();
    }

    bool isLeft(bool pressed = false) {
      return pressed ? _encoder.isLeftH() : _encoder.isLeft();
    }
    
    bool isRight(bool pressed = false) {
      return pressed ? _encoder.isRightH() : _encoder.isRight();
    }

    bool isClick() {
      return _encoder.isClick();
    }

    bool isAnyTurn() {
      return _encoder.isTurn();
    }

  private:
    Encoder _encoder;
};
