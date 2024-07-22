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
bool earlyClick = true;  // hack to disable false clicks on startup
uint8_t cursorPosition = 0;
uint8_t cursorLimit = 0;
RelayCommand manualOverrideCommand = None;

uint8_t localMinutes;
uint8_t localHours;
uint8_t localDay;
uint8_t localMonth;
uint16_t localYear;

MemoryEntry mem_currentMode(64);
MemoryEntry mem_scheduleOnTime(0);
MemoryEntry mem_scheduleOffTime(2);
MemoryEntry mem_d2dDuskOffset(4);
MemoryEntry mem_d2dDawnOffset(6);
MemoryEntry mem_sleepTimer(8);
MemoryEntry mem_remoteEnabled(10);
MemoryEntry mem_displaySleepEnabled(12);
MemoryEntry mem_soundEnabled(14);

Mode currentMode = mem_currentMode.read();
uint16_t scheduleOnTime = mem_scheduleOnTime.read();
uint16_t scheduleOffTime = mem_scheduleOffTime.read();
int16_t d2dDuskOffset = mem_d2dDuskOffset.read();
int16_t d2dDawnOffset = mem_d2dDawnOffset.read();
uint16_t sleepTimer = mem_sleepTimer.read();
uint16_t remoteEnabled = mem_remoteEnabled.read();
uint16_t displaySleepEnabled = mem_displaySleepEnabled.read();
uint16_t soundEnabled = mem_soundEnabled.read();

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

void setLocalTime() {
  localMinutes = rtc.getMinutes();
  localHours = rtc.getHours();
  localDay = rtc.getDay();
  localMonth = rtc.getMonth();
  localYear = rtc.getYear();
  bootTime = rtc.unix();
}

void saveSettings() {
  logger.print(F("Saving settings..."));
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
    case AdditionalSettings: {
      mem_remoteEnabled.write(remoteEnabled);
      mem_displaySleepEnabled.write(displaySleepEnabled);
      mem_soundEnabled.write(soundEnabled);
      break;
    }
    case TimeSettings:
    case DateSettings: {
      rtc.setTime(0, localMinutes, localHours, localDay, localMonth, localYear);
      setLocalTime();
      break;
    }
  }
}

void handleKnobRotation(bool direction, bool pressed = false) {
  logger.print("Current page: " + String(currentPage));
  if (displaySleeping) return;

  if (pressed) {
    buzzer.beep(6);
    switch (currentPage) {
      case Home: {
        adjustValue(currentMode, Manual, SleepTimer, direction == Left, 1);
        break;
      }
      case ScheduleSettings: {
        adjustValue(cursorPosition == 0 ? scheduleOnTime : scheduleOffTime, 0, 1430, direction == Left, 10, direction != Left);
        break;
      }
      case Dusk2DawnSettings: {
        adjustValue(cursorPosition == 0 ? d2dDuskOffset : d2dDawnOffset, -60, 60, direction == Left, 1);
        break;
      }
      case SleepTimerSettings: {
        adjustValue(sleepTimer, 10, 360, direction == Left, 10);
        break;
      }
      case AdditionalSettings: {
        adjustValue(cursorPosition == 0 ? remoteEnabled : (cursorPosition == 1 ? displaySleepEnabled : soundEnabled), 0, cursorPosition == 1 ? 2 : 1, direction == Left, 1);
        break;
      }
      case TimeSettings: {
        adjustValue(cursorPosition == 0 ? localHours : localMinutes, 0, cursorPosition == 1 ? 59 : 23, direction == Left, 1);
        break;
      }
      case DateSettings: {
        adjustValue(cursorPosition == 0 ? (uint16_t&)localDay : (cursorPosition == 1 ? (uint16_t&)localMonth : localYear), cursorPosition == 2 ? 2000 : 1, cursorPosition == 0 ? 30 : (cursorPosition == 1 ? 12 : 2100), direction == Left, 1);
        break;
      }
    }
  } else if (setupMode) {
    adjustValue(cursorPosition, 0, cursorLimit, direction == Left, 1);
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
  if (earlyClick) {
    logger.print(F("[Knob] early click"));
    earlyClick = true;
    return;
  }
  logger.print(F("[Knob] click"));
  if (currentPage == Home) {
    toggleRelay();
    manualOverrideCommand = relay.getState() ? TurnOn : TurnOff;
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
      String date = "DATE: " + String(rtc.getDate(true));
      display.renderLayout(Display::List, true, PAGE_NAMES[page], uptime, temp, date);
      break;
    }
    case Home: {
      String title = homeWithTime ? rtc.getTime(true) : PAGE_NAMES[page];
      String upcoming = getUpcomingCommandText();
      display.clear(false);
      printRelayStatus();
      if (setupMode) display.printCursor(Display::ListWithBigTitle);
      display.renderLayout(Display::ListWithBigTitle, false, title, MODE_NAMES[currentMode], upcoming);
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
      String duskOffset = "DUSK OFFSET: " + String(d2dDuskOffset);
      String dawnOffset = "DAWN OFFSET: " + String(d2dDawnOffset);
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
      String timer = "TIMER: " + String(sleepTimer);
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
      String remote = "REMOTE: " + String(ON_OFF_NAMES[remoteEnabled]);
      String displaySleep = "SCREEN SLEEP: " + String(ON_OFF_NAMES[displaySleepEnabled]);
      String sound = "SOUND: " + String(ON_OFF_NAMES[soundEnabled]);
      String title = PAGE_NAMES[page];
      if (setupMode) {
        title += SETUP_MODE_MARKER;
        display.clear(false);
        display.printCursor(Display::List, cursorPosition);
      }
      display.renderLayout(Display::List, !setupMode, title, remote, displaySleep, sound);
      break;
    }
    case TimeSettings: {
      cursorLimit = 1;
      String hour = "H: " + String(localHours);
      String minute = "M: " + String(localMinutes);
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
      cursorLimit = 2;
      String day = "D: " + String(localDay);
      String month = "M: " + String(localMonth);
      String year = "Y: " + String(localYear);
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

void runRelayCommand(uint8_t command) {
  if (command == TurnOn) {
    relay.on();
    led.on();
    manualOverrideCommand = None;
  } else if (command == TurnOff) {
    relay.off();
    led.off();
    manualOverrideCommand = None;
  }
}

void resolveRelayCommand(uint8_t automaticCommand) {
  if (manualOverrideCommand == None) {
    runRelayCommand(automaticCommand);
  } else if (manualOverrideCommand == automaticCommand || automaticCommand == None) {
    manualOverrideCommand = None;
  }
}

String getUpcomingCommandText() {
  uint16_t currentMsm = rtc.minutesSinceMidnight();
  String result = "";

  switch (currentMode) {
    case Schedule: {
      UpcomingCommand upcomingCommand = Schedule::getUpcomingCommand(currentMsm, scheduleOnTime, scheduleOffTime, relay.getState());
      if (upcomingCommand.command == TurnOn) {
        result += "On in ";
        result += rtc.msmToString(upcomingCommand.msm);
      } else {
        result += "Off in ";
        result += rtc.msmToString(upcomingCommand.msm);
      }
      break;
    }
  }

  return result;
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
      logger.print(String(BUILD_SEC) + " " + String(BUILD_MIN) + " " + String(BUILD_HOUR) + " " + String(BUILD_DAY) + " " + String(BUILD_MONTH) + " " + String(BUILD_YEAR));
      rtc.setCompileTime();
    }

    logger.print("[RTC] time: '" + rtc.getTime() + "'");
    logger.print("[RTC] date: '" + rtc.getDate() + "'");

    setLocalTime();

    bootTime = rtc.unix();
  }

  display.begin();

  #ifdef CLEAR_EEPROM
    mem_currentMode.write(0);
    mem_scheduleOnTime.write(0);
    mem_scheduleOffTime.write(0);
    mem_d2dDuskOffset.write(0);
    mem_d2dDawnOffset.write(0);
    mem_sleepTimer.write(0);
    mem_remoteEnabled.write(0);
    mem_displaySleepEnabled.write(0);
    mem_soundEnabled.write(0);
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

    earlyClick = false;
    timer_pageRender = current;
  }

  if (current - timer_pageReset >= 10000) {
    uint16_t currentMsm = rtc.minutesSinceMidnight();

    switch (currentMode) {
      case Schedule: {
        RelayCommand scheduleCommand = Schedule::getCommand(currentMsm, scheduleOnTime, scheduleOffTime, relay.getState());
        // logger.print(scheduleCommand);
        resolveRelayCommand(scheduleCommand);
        break;
      }
    }

    if (counter_displaySleep < 2) {
      counter_displaySleep++;
      displaySleeping = false;
    } else if (!displaySleeping) {
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
