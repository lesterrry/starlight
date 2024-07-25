enum Page {
  Version,
  Info,
  Home,
  ScheduleSettings,
  DuskToDawnSettings,
  SleepTimerSettings,
  AdditionalSettings,
  TimeSettings,
  DateSettings
};

enum Mode {
  Manual,
  Schedule,
  DuskToDawn,
  SleepTimer
};

enum Direction {
  Left,
  Right
};

enum RelayCommand {
  None,
  TurnOn,
  TurnOff
};

struct UpcomingCommand {
  RelayCommand command;
  uint16_t msm;
};

struct SolarTime {
  uint16_t sunriseMsm;
  uint16_t sunsetMsm;
};

const char* PAGE_NAMES[] = {"VERSION", "INFO", "HOME", "SCHEDULE", "D2D", "SLEEP", "SETTINGS", "TIME", "DATE"};

const char* MODE_NAMES[] = {"MANUAL", "SCHEDULE", "D2D", "SLEEP"};

const char* ON_OFF_NAMES[] = {"On", "Off", "Relay"};

const String BUILD_DATE = (BUILD_DAY > 9 ? "" : "0") + String(BUILD_DAY) + "." + (BUILD_MONTH > 9 ? "" : "0") + String(BUILD_MONTH) + "." + String(BUILD_YEAR);

const char SETUP_MODE_MARKER = ':';
