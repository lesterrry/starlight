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
bool pageSwitched = false;
bool modeSwitched = false;
bool homeWithTime = false;
bool displaySleeping = false;
bool setupMode = false;
bool knobDown = false;
uint8_t cursorPosition = 0;
uint8_t cursorLimit = 0;

MemoryEntry mem_currentMode(64);
MemoryEntry mem_scheduleOnTime(0);
MemoryEntry mem_scheduleOffTime(2);
MemoryEntry mem_d2dDuskOffset(4);
MemoryEntry mem_d2dDawnOffset(6);
MemoryEntry mem_sleepTimer(8);

Mode currentMode = mem_currentMode.read();
uint16_t scheduleOnTime = mem_scheduleOnTime.read();
uint16_t scheduleOffTime = mem_scheduleOffTime.read();
int16_t d2dDuskOffset = mem_d2dDuskOffset.read();
int16_t d2dDawnOffset = mem_d2dDawnOffset.read();
uint16_t sleepTimer = mem_sleepTimer.read();

unsigned long timer_pageRender = 0;
unsigned long timer_pageReset = 0;
unsigned long counter_knobDown = 0;
uint8_t counter_displaySleep = 0;

void(* reset) (void) = 0;

template <typename T>
void adjustValue(T &value, int min, int max, bool decrement = false, int step = 10, bool inclusiveBoundary = false) {
  if (decrement) {
    int16_t adjusted = value - step;
    bool overflow = inclusiveBoundary ? adjusted <= min : adjusted < min;
    value = overflow ? max : adjusted;
  } else {
    int16_t adjusted = value + step;
    bool overflow = inclusiveBoundary ? adjusted >= max : adjusted > max;
    value = overflow ? min : adjusted;
  }
}

void saveSettings() {
  switch (currentPage) {
    case ScheduleSettings: {
      mem_scheduleOnTime.write(scheduleOnTime);
      mem_scheduleOffTime.write(scheduleOffTime);
      break;
    }
    case Dusk2DawnSettings: {
      mem_d2dDuskOffset.write(d2dDuskOffset);
      mem_d2dDawnOffset.write(d2dDawnOffset);
      break;
    }
    case SleepTimerSettings: {
      mem_sleepTimer.write(sleepTimer);
      break;
    }
  }
}

void handleKnobRotation(bool direction, bool pressed = false) {
  logger.print("Current page: " + String(currentPage));
  if (displaySleeping) return;

  if (pressed) {
    switch (currentPage) {
      case Home: {
        if (direction == Left) {
          buzzer.beep(5);
          adjustValue(currentMode, Manual, SleepTimer, true, 1);
        } else {
          buzzer.beep(6);
          adjustValue(currentMode, Manual, SleepTimer, false, 1);
        }
        break;
      }
      case ScheduleSettings: {
        if (direction == Left) {
          buzzer.beep(5);
          if (cursorPosition == 0) {
            adjustValue(scheduleOnTime, 0, MSM_IN_DAY, true, 10, false);
          } else {
            adjustValue(scheduleOffTime, 0, MSM_IN_DAY, true, 10, false);
          }
        } else {
          buzzer.beep(6);
          if (cursorPosition == 0) {
            adjustValue(scheduleOnTime, 0, MSM_IN_DAY, false, 10, true);
          } else {
            adjustValue(scheduleOffTime, 0, MSM_IN_DAY, false, 10, true);
          }
        }
      }
      case Dusk2DawnSettings: {
        if (direction == Left) {
          buzzer.beep(5);
          if (cursorPosition == 0) {
            adjustValue(d2dDuskOffset, -60, 60, true, 1);
          } else {
            adjustValue(d2dDawnOffset, -60, 60, true, 1);
          }
        } else {
          buzzer.beep(6);
          if (cursorPosition == 0) {
            adjustValue(d2dDuskOffset, -60, 60, false, 1);
          } else {
            adjustValue(d2dDawnOffset, -60, 60, false, 1);
          }
        }
      }
      case SleepTimerSettings: {
        if (direction == Left) {
          buzzer.beep(5);
          adjustValue(sleepTimer, 10, 360, true, 10);
        } else {
          buzzer.beep(6);
          adjustValue(sleepTimer, 10, 360, false, 10);
        }
      }
    }
  } else if (setupMode) {
    if (direction == Left) {
      buzzer.beep(5);
      adjustValue(cursorPosition, 0, cursorLimit, true, 1);
    } else {
      buzzer.beep(6);
      adjustValue(cursorPosition, 0, cursorLimit, false, 1);
    }
  } else {
    if (direction == Left) {
      if (currentPage <= Version) return;
      currentPage = currentPage - 1;
    } else {
      if (currentPage >= DateSettings) return;
      currentPage = currentPage + 1;
    }
  }
}

void handleKnobPress(bool pressed) {
  if (currentPage == Home) {
    if (pressed) {
      setupMode = true;
    } else {
      setupMode = false;
    }
  }
}

void handleKnobClick() {
  logger.print(F("[Knob] click"));
  if (currentPage == Home) {
    toggleRelay();
  } else if (currentPage > Home) {
    if (!setupMode) {
      buzzer.beep(6, 100);
    } else {
      buzzer.playToneB();
      saveSettings();
    }
    cursorPosition = 0;
    setupMode = !setupMode;
  } else if (currentPage == Version) {
    display.printTitle(F("REBOOTING"));
    display.render();
    delay(500);
    reset();
  }
}

void handleRemoteClick() {
  logger.print(F("[Radio] click"));
  toggleRelay();
}

void renderPage(uint8_t page) {
  logger.print("Rendering page " + String(page));
  switch (page) {
    case Version: {
      String version = String(VERSION) + " (" + String(BUILD_DATE) + ")";
      display.renderLayout(Display::List, true, PAGE_NAMES[page], "", "", version);
      break;
    }
    case Info: {
      String uptime = "UP: " + rtc.getUnixDelta(bootTime);
      String temp = "TEMP: " + String(rtc.getTemp());
      String msm = "MSM: " + String(rtc.minutesSinceMidnight());
      display.renderLayout(Display::List, true, PAGE_NAMES[page], uptime, temp, msm);
      break;
    }
    case Home: {
      String title = homeWithTime ? rtc.getTime(true) : PAGE_NAMES[page];
      display.clear(false);
      printRelayStatus();
      if (setupMode) display.printCursor(Display::ListWithBigTitle);
      display.renderLayout(Display::ListWithBigTitle, false, title, MODE_NAMES[currentMode]);
      break;
    }
    case ScheduleSettings: {
      cursorLimit = 1;
      String on = "ON:  " + rtc.getTimeFromMsm(scheduleOnTime);
      String off = "OFF: " + rtc.getTimeFromMsm(scheduleOffTime);
      String title = PAGE_NAMES[page];
      if (setupMode) {
        title += SETUP_MODE_MARKER;
        display.clear(false);
        display.printCursor(Display::List, cursorPosition);
      }
      display.renderLayout(Display::List, !setupMode, title, on, off);
      break;
    }
    case Dusk2DawnSettings: {
      cursorLimit = 1;
      String duskOffset = "Dusk Offset: " + String(d2dDuskOffset);
      String dawnOffset = "Dawn Offset: " + String(d2dDawnOffset);
      String title = PAGE_NAMES[page];
      if (setupMode) {
        title += SETUP_MODE_MARKER;
        display.clear(false);
        display.printCursor(Display::List, cursorPosition);
      }
      display.renderLayout(Display::List, !setupMode, title, duskOffset, dawnOffset);
      break;
    }
    case SleepTimerSettings: {
      cursorLimit = 0;
      String timer = "Timer: " + String(sleepTimer);
      String title = PAGE_NAMES[page];
      if (setupMode) {
        title += SETUP_MODE_MARKER;
        display.clear(false);
        display.printCursor(Display::List, cursorPosition);
      }
      display.renderLayout(Display::List, !setupMode, title, timer);
      break;
    }
    case AdditionalSettings: {
      cursorLimit = 2;
      String remote = "Remote: ";
      String displaySleep = "Display Sleep: ";
      String beep = "Sound: ";
      String title = PAGE_NAMES[page];
      if (setupMode) {
        title += SETUP_MODE_MARKER;
        display.clear(false);
        display.printCursor(Display::List, cursorPosition);
      }
      display.renderLayout(Display::List, !setupMode, title, remote, displaySleep, beep);
      break;
    }
    case TimeSettings: {
      cursorLimit = 1;
      String hour = "H: ";
      String minute = "M: ";
      String title = PAGE_NAMES[page];
      if (setupMode) {
        title += SETUP_MODE_MARKER;
        display.clear(false);
        display.printCursor(Display::List, cursorPosition);
      }
      display.renderLayout(Display::List, !setupMode, title, hour, minute);
      break;
    }
    case DateSettings: {
      cursorLimit = 1;
      String day = "D: ";
      String month = "M: ";
      String year = "Y: ";
      String title = PAGE_NAMES[page];
      if (setupMode) {
        title += SETUP_MODE_MARKER;
        display.clear(false);
        display.printCursor(Display::List, cursorPosition);
      }
      display.renderLayout(Display::List, !setupMode, title, day, month, year);
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

  led.off();

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

  #ifdef CLEAR_EEPROM
    mem_currentMode.write(0);
    mem_scheduleOnTime.write(0);
    mem_scheduleOffTime.write(0);
  #endif
  #ifndef QUICK_BOOT
    display.printTitle(F("SUBURBS:"), F("SUNSET"));
    buzzer.playToneB();
    delay(2000);
    display.printTitle("", String(VERSION) + "      " + String(BUILD_DATE));
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
    display.printTitle("DEBUG");
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
  
  knob.reset();
}

void loop() {
  unsigned long current = millis();

  knob.update();

  pageSwitched = false;
  modeSwitched = false;

  if (radio.isReceiving()) {
    handleRemoteClick();
  }

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
    if (counter_knobDown > ENC_HOLD_DELAY) {
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
    if (mem_currentMode.read() != currentMode) mem_currentMode.write(currentMode);
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
      saveSettings();
      display.clear();
      displaySleeping = true;
      setupMode = false;
    }

    if (currentPage != Home) {
      saveSettings();
      setupMode = false;
      currentPage = Home;
      renderPage(currentPage);
    }

    timer_pageReset = current;
  }
}
