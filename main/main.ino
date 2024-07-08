#define DEBUG 1
#define SET_RTC_COMPILE_TIME 0

#define VERSION "0.1.0"

#define RELAY_PIN      7
#define RADIO_PIN      31
#define OLED_MOSI_PIN  35
#define OLED_CLK_PIN   37
#define OLED_DC_PIN    39
#define OLED_CS_PIN    41
#define OLED_RESET_PIN 43

#include <microDS3231.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
