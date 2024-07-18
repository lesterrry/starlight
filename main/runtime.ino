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
bool modeSwitched = false;
bool homeWithTime = false;
bool displaySleeping = false;
bool cursorVisible = false;
bool knobDown = false;
uint8_t cursorPosition = 0;

unsigned long timer_pageRender = 0;
unsigned long timer_pageReset = 0;
unsigned long counter_knobDown = 0;
uint8_t counter_displaySleep = 0;

void handleKnobRotation(bool direction, bool pressed = false) {
  logger.print("Current page: " + String(currentPage));
  if (displaySleeping) return;

  if (pressed) {
    if (currentPage == Home) {
      if (direction == Left) {
        buzzer.beep(5);
        currentMode = currentMode <= Manual ? SleepTimer : currentMode - 1;
      } else {
        buzzer.beep(6);
        currentMode = currentMode >= SleepTimer ? Manual : currentMode + 1;
      }
    }
  } else {
    if (direction == Left) {
      if (currentPage <= Info) return;
      currentPage = currentPage - 1;
    } else {
      if (currentPage >= TimeSettings) return;
      currentPage = currentPage + 1;
    }
  }
}

void handleKnobPress(bool pressed) {
  if (pressed) {
    cursorVisible = true;
  } else {
    cursorVisible = false;
  }
}

void handleKnobClick() {
  if (currentPage == Home) {
    toggleRelay();
  }
}

void renderPage(uint8_t page) {
  logger.print("Rendering page " + String(page));
  switch (page) {
    case Info: {
      String uptime = "UP: " + rtc.getUnixDelta(bootTime);
      String temp = "TEMP: " + String(rtc.getTemp());
      String version = String(VERSION) + " (" + String(BUILD_DATE) + ")";
      display.renderLayout(Display::List, true, PAGE_NAMES[page], uptime, temp, version);
      break;
    }
    case Home: {
      display.clear(false);
      printRelayStatus();
      if (cursorVisible) display.printCursor(Display::ListWithBigTitle);
      display.renderLayout(Display::ListWithBigTitle, false, homeWithTime ? rtc.getTime(true) : PAGE_NAMES[page], MODE_NAMES[currentMode]);
      break;
    }
    case ScheduleSettings: {
      String on = "ON:  ";
      String off = "OFF: ";
      display.renderLayout(Display::List, true, PAGE_NAMES[page], on, off);
      break;
    }
    default: {
      display.renderLayout(Display::List, true, PAGE_NAMES[page]);
    }
  }
}

void printRelayStatus() {
  if (relay.getState()) {
    display.printCornerChar('I');
  } else {
    display.printCornerChar('O');
  }
}

void toggleRelay() {
  led.toggle();
  relay.toggle();
  buzzer.beep(6, 100, !relay.getState());
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
  modeSwitched = false;

  if (knob.isRight()) {
    buzzer.beep(4);
    handleKnobRotation(Right);
    pageSwitched = true;
  } else if (knob.isLeft()) {
    buzzer.beep(3);
    handleKnobRotation(Left);
    pageSwitched = true;
  } else if (knob.isClick()) {
    handleKnobClick();
    modeSwitched = true;
  } else if (knob.isDown()) {
    counter_knobDown++;
    if (counter_knobDown > 1750) {
      if (knob.isRight(true)) {
        handleKnobRotation(Right, true);
        modeSwitched = true;
      } else if (knob.isLeft(true)) {
        handleKnobRotation(Left, true);
        modeSwitched = true;
      }
      if (!knobDown) {
        handleKnobPress(true);
        modeSwitched = true;
      }
      knobDown = true;
    }
  } else if (knobDown) {
    handleKnobPress(false);
    knobDown = false;
    modeSwitched = true;
    counter_knobDown = 0;
  } else {
    counter_knobDown = 0;
  }

  if (pageSwitched || modeSwitched) {
    timer_pageReset = current;
    timer_pageRender = current;
    counter_displaySleep = 0;
    displaySleeping = false;
    if (pageSwitched) homeWithTime = false;
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
