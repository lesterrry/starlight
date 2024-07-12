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
Mode currentMode = Manual;
bool pageSwitched = false;
bool homeWithTime = false;
bool displaySleeping = false;
bool knobDown = false;

unsigned long timer_pageRender = 0;
unsigned long timer_pageReset = 0;
uint8_t counter_displaySleep = 0;

void handleKnobRotation(bool direction) {
  logger.print("Current page: " + String(currentPage));
  if (displaySleeping) return;

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
      String uptime = "UP: " + rtc.getUnixDelta(bootTime);
      String temp = "TEMP: " + String(rtc.getTemp());
      String version = String(VERSION) + " (" + String(BUILD_DATE) + ")";
      display.renderLayout(Display::List, PAGE_NAMES[page], uptime, temp, version);
      break;
    }
    case Home: {
      display.renderLayout(Display::ListWithBigTitle, homeWithTime ? rtc.getTime(true) : PAGE_NAMES[page], "MANUAL");
      break;
    }
    default: {
      display.renderLayout(Display::List, PAGE_NAMES[page]);
    }
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
    display.renderTitle("", String(VERSION) + "      " + String(BUILD_DATE));
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

  knob.reset();

  logger.print(F("Entering loop..."));
}

void loop() {
  unsigned long current = millis();

  knob.update();

  pageSwitched = false;

  if (knob.isRight()) {
    buzzer.beep(4);
    handleKnobRotation(Right);
    pageSwitched = true;
  } else if (knob.isLeft()) {
    buzzer.beep(3);
    handleKnobRotation(Left);
    pageSwitched = true;
  } else if (knob.isClick()) {
    buzzer.beep(6, 100);
    pageSwitched = true;
  } else if (knob.isDown()) {
    if (knob.isRight(true)) {
      logger.print("right");
    } else if (knob.isLeft(true)) {
      logger.print("left");
    }
    knobDown = true;
  } else if (knobDown) {
    knobDown = false;
  }

  if (pageSwitched) {
    timer_pageReset = current;
    timer_pageRender = current;
    counter_displaySleep = 0;
    displaySleeping = false;
    homeWithTime = false;
    renderPage(currentPage);
  }

  if (current - timer_pageRender >= 1000 && !displaySleeping) {
    bool needsRerender = false;
    if (currentPage == Info) {
      needsRerender = true;
    } else if (currentPage == Home) {
      homeWithTime = true;
      needsRerender = true;
    }

    if(needsRerender) renderPage(currentPage);

    timer_pageRender = current;
  }

  if (current - timer_pageReset >= 10000) {
    if (counter_displaySleep < 2) {
      counter_displaySleep++;
      displaySleeping = false;
    } else {
      display.clear();
      displaySleeping = true;
    }

    if (currentPage != Home) {
      currentPage = Home;
      renderPage(currentPage);
    }

    timer_pageReset = current;
  }

}