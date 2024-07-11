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

};

enum Direction {
  Left,
  Right
};

const char* PAGE_NAMES[] = {"INFO", "HOME", "SCHEDULE", "D2D", "SLEEP", "SETTINGS", "TIME"};

const String BUILD_DATE = String(BUILD_DAY) + "." + String(BUILD_MONTH) + "." + String(BUILD_YEAR);
