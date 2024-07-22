class Schedule {
  public:
    static RelayCommand getCommand(uint16_t currentMsm, uint16_t onMsm, uint16_t offMsm, bool isOn) {
      if (onMsm < offMsm) {
        if (currentMsm > onMsm && currentMsm < offMsm) {
          return isOn ? None : TurnOn;
        } else {
          return isOn ? TurnOff : None;
        }
      } else {
        if (currentMsm > onMsm || currentMsm < offMsm) {
          return isOn ? None : TurnOn;
        } else {
          return isOn ? TurnOff : None;
        }
      }
    }

    static UpcomingCommand getUpcomingCommand(uint16_t currentMsm, uint16_t onMsm, uint16_t offMsm, bool isOn) {
      uint16_t timeToOn, timeToOff;

      if (currentMsm < onMsm) timeToOn = onMsm - currentMsm;
      else timeToOn = 1440 + onMsm - currentMsm;

      if (currentMsm < offMsm) timeToOff = offMsm - currentMsm;
      else timeToOff = 1440 + offMsm - currentMsm;

      UpcomingCommand result;
      if (!isOn) {
        result.msm = timeToOn;
        result.command = TurnOn;
      } else {
        result.msm = timeToOff;
        result.command = TurnOff;
      }
      
      return result;
    }
};
