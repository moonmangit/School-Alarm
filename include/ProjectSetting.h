#ifndef __PROJECTSETTING__H__
#define __PROJECTSETTING__H__

/*Custom*/
#include "Button.h"
#include "DisplayHelper.h"
#include "Led.h"
#include "ProjectDataType.h"
#include "SerialMP3Player.h"

/*External*/
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include <ESP32Time.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <ParenthLogger.h>
#include <WiFi.h>
#include <esp_task_wdt.h>

/*Connection*/
#define WIFI_SSID "wifi-Seagame"
#define WIFI_PSK "123456789"
#define TIME_SERVER "https://www.timeapi.io/api/Time/current/zone?timeZone=Asia/Bangkok"
#define TIME_SERVER_ATTEMP 5
#define OTA_AUTH_KEY "bb0ea5d40d5897a0fc48aac9736ede19"

/*Serial*/
#define MONITOR_SPEED 115200
#define __DATE_F__ "%B %d %Y"
#define __TIME_F__ "%H:%M:%S"

/*Led*/
#define PIN_LED_ERR 26

/*Button*/
#define PIN_BUTTON_TOP 32
#define PIN_BUTTON_MID 33
#define PIN_BUTTON_BOT 25

/*Mp3*/
#define PIN_MP3_TX 16
#define PIN_MP3_RX 17
#define MP3_SPEED 9600
#define MP3_VOL 30

/*Display SSD1306*/
#define SSD_ADDR 0x3c
#define SSD_WIDTH 128
#define SSD_HEIGHT 32

/*Watchdog*/
#define WDT_HANDLE_SEC 3

/*Sound tasks*/
SoundTask tasks[] = {
    {{8, 0, 0}, 1, 10000, "morning event"},
    {{8, 15, 0}, 2, 10000, "after morning event"},
    {{8, 30, 0}, 3, 10000, "class 1"},
    {{9, 30, 0}, 4, 10000, "class 2"},
    {{10, 30, 0}, 5, 10000, "class 3"},
    {{11, 30, 0}, 5, 10000, "launch"},
    {{12, 30, 0}, 6, 10000, "class 4"},
    {{13, 30, 0}, 7, 10000, "class 5"},
    {{14, 30, 0}, 8, 10000, "class 6"},
    {{15, 30, 0}, 9, 10000, "class 7"},
    {{16, 30, 0}, 10, 10000, "class 7"},
    {{17, 0, 0}, 11, 5000, "school end"}};
int tasksLength = sizeof(tasks) / sizeof(tasks[0]);

#endif //!__PROJECTSETTING__H__