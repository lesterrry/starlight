/******************************
Suburbs: sunset
Interactive time-sensitive relay

me@aydar.media
******************************/

#define DEBUG 0
#define SET_RTC_COMPILE_TIME 0

#define VERSION "0.1.2"

#undef DEBUG_TONES
#undef DEBUG_MODULES
#undef DEBUG_D2D
#undef QUICK_BOOT
#undef CLEAR_EEPROM
#undef FASTLED_INTERNAL

#define RELAY_PIN      7
#define RADIO_PIN      31
#define OLED_MOSI_PIN  35
#define OLED_CLK_PIN   37
#define OLED_DC_PIN    39
#define OLED_CS_PIN    41
#define OLED_RESET_PIN 43
#define ENC_CLK_PIN    45
#define ENC_DT_PIN     47
#define ENC_SW_PIN     49
#define ENC_HOLD_DELAY 2500
#define BUZZER_PIN     53
#define LED_PIN        27
#define LED_NUMBER     8
#define LED_BRIGHTNESS 100
#define LONGITUDE      56.237472
#define LATITUDE       36.841194
#define TIMEZONE       3

#include <microDS3231.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <GyverEncoder.h>
#include <FastLED.h>
#include <Dusk2Dawn.h>
#include <EEPROM.h>
