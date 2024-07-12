enum Page {
  Info,
  Home,
  ScheduleSettings,
  Dusk2DawnSettings,
  SleepTimerSettings,
  AdditionalSettings,
  TimeSettings
};

enum Mode {
  Manual,
  Schedule,
  Dusk2Dawn,
  SleepTimer
};

enum Direction {
  Left,
  Right
};

const char* PAGE_NAMES[] = {"INFO", "HOME", "SCHEDULE", "D2D", "SLEEP", "SETTINGS", "TIME"};

const char* MODE_NAMES[] = {"MANUAL", "SCHEDULE", "D2D", "SLEEP"};

const String BUILD_DATE = (BUILD_DAY > 9 ? "" : "0") + String(BUILD_DAY) + "." + (BUILD_MONTH > 9 ? "" : "0") + String(BUILD_MONTH) + "." + String(BUILD_YEAR);
