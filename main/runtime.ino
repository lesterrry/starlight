Logger logger(DEBUG);
Relay relay(RELAY_PIN);
Radio radio(RADIO_PIN);
RTC rtc;

void setup() {
  logger.begin();
  logger.print(F("Suburbs Sunset initializing..."));

  if (!rtc.isConnected()) {
    logger.print(F("[RTC] Not connected"));
  } else {
    if (SET_RTC_COMPILE_TIME) {
      logger.print(F("[RTC] Setting compile time..."));
      rtc.setCompileTime();
    }

    logger.print("[RTC] time: '" + rtc.getTime() + "'");
    logger.print("[RTC] date: '" + rtc.getDate() + "'");
  }
}

void loop() {
  // int isRadioReceiving = radio.isReceiving();


  // logger.print(rtc.isConnected());

  delay(1000);
}