Logger logger(DEBUG);
Relay relay(RELAY_PIN);
Radio radio(RADIO_PIN);
RTC rtc;
Display display(OLED_MOSI_PIN, OLED_CLK_PIN, OLED_DC_PIN, OLED_RESET_PIN, OLED_CS_PIN);

void setup() {
  logger.begin();
  logger.print(F("Suburbs Sunset initializing..."));
  logger.print(VERSION);

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

  display.begin();
  display.renderTitle(F("SUBURBS:"), F("SUNSET"));
  delay(2000);
  display.renderTitle("", VERSION);
  delay(1000);
  display.clear();
}

void loop() {
  // int isRadioReceiving = radio.isReceiving();


  // logger.print(rtc.isConnected());

  delay(1000);
}