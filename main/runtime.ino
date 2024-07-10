Logger logger(DEBUG);
Relay relay(RELAY_PIN);
Radio radio(RADIO_PIN);
RTC rtc;
Display display(OLED_MOSI_PIN, OLED_CLK_PIN, OLED_DC_PIN, OLED_RESET_PIN, OLED_CS_PIN);
Knob knob(ENC_CLK_PIN, ENC_DT_PIN, ENC_SW_PIN);
Buzzer buzzer(BUZZER_PIN);
LED led(LED_NUMBER, LED_BRIGHTNESS);

uint32_t bootTime;
Page currentPage = Home;
bool actionWasMade = false;

void handleKnobRotation(bool direction) {
  logger.print("Current page: " + String(currentPage));
  if (direction == Left) {
    if (currentPage <= Info) return;
    currentPage = currentPage - 1;
  } else {
    if (currentPage >= TimeSettings) return;
    currentPage = currentPage + 1;
  }
}

void renderPage(uint8_t page) {
  switch (page) {
    case Info: {
      String uptime = "UP " + rtc.getUnixDelta(bootTime);
      display.renderLayout(Display::List, PAGE_NAMES[page], uptime);
      break;
    }
    default:
      display.renderLayout(Display::List, PAGE_NAMES[page]);
  }
}

void setup() {
  logger.begin();
  logger.print(F("Sunset initializing..."));
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

    bootTime = rtc.unix();
  }

  display.begin();

  #ifndef QUICK_BOOT
    display.renderTitle(F("SUBURBS:"), F("SUNSET"));
    buzzer.playToneB();
    delay(2000);
    display.renderTitle("", VERSION);
    delay(1000);
    display.clear();
  #endif
  #ifdef DEBUG_TONES
		int i = 50;
		while(true) {
			logger.print(i);
			buzzer.sound(i);
			delay(1000);
			i += 50;
		}
	#endif
  #ifdef DEBUG_MODULES
    led.light(CRGB::Purple);
    display.renderTitle("DEBUG");
    logger.print("Relay switching on...");
    relay.on();
    delay(1000);
    logger.print("Relay switching off...");
    relay.off();

		while(true) {
      knob.update();

      if (radio.isReceiving()) {
        logger.print("Radio: receiving");
      }

      if (knob.isRight()) {
        logger.print("Knob: left");
      } else if (knob.isLeft()) {
        logger.print("Knob: right");
      } else if (knob.isClick()) {
        logger.print("Knob: click");
      }
		}
	#endif

  renderPage(currentPage);

  logger.print(F("Entering loop..."));
}

void loop() {
  knob.update();

  actionWasMade = false;

  if (knob.isRight()) {
    buzzer.beep(4);
    handleKnobRotation(Right);
    actionWasMade = true;
  } else if (knob.isLeft()) {
    buzzer.beep(3);
    handleKnobRotation(Left);
    actionWasMade = true;
  } else if (knob.isClick()) {
    buzzer.beep(6, 100);
    actionWasMade = true;
  }

  if (actionWasMade) {
    renderPage(currentPage);
  }
}