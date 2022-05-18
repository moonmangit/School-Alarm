#include "Button/Button.h"
#include "DisplayHelper/CustomFonts.h"
#include "DisplayHelper/DisplayHelper.h"
#include "Led/Led.h"
#include "ProjectDataType.h"
#include "ProjectSetting.h"
#include "SerialMp3Player/SerialMP3Player.h"
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
#include <WiFiUdp.h>
#include <esp_task_wdt.h>
#define __DATE_F__ "%B %d %Y"
#define __TIME_F__ "%H:%M:%S"

// ========================================================================== //
// assets
const SoundTask tasks[] = {
    //! manual sort needed
    {{8, 0, 0}, 1, 10, "morning event"},
    {{8, 15, 0}, 2, 10, "after morning event"},
    {{8, 30, 0}, 3, 10, "class 1"},
    {{9, 30, 0}, 4, 10, "class 2"},
    {{10, 30, 0}, 5, 10, "class 3"},
    {{11, 30, 0}, 5, 10, "launch"},
    {{12, 30, 0}, 6, 10, "class 4"},
    {{13, 30, 0}, 7, 10, "class 5"},
    {{14, 30, 0}, 8, 10, "class 6"},
    {{15, 30, 0}, 9, 10, "class 7"},
    {{16, 30, 0}, 10, 10, "class 7"},
    {{17, 0, 0}, 11, 5, "school end"}};
const uint16_t taskLen = sizeof(tasks) / sizeof(tasks[0]);
uint16_t taskIndex = 0;
ESP32Time rtc;
SerialMP3Player mp3;
Led errorLed(PIN_LED_1);
Button topButton(PIN_BUTTON_1);
Button midButton(PIN_BUTTON_2);
Button botButton(PIN_BUTTON_3);
Adafruit_SSD1306 ssd(128, 32, &Wire, -1, 400000UL, 100000UL);
DisplayHelper display(&ssd);
/* mode handler */
void (*mode_service)();
void (*mode_setup)();
namespace render_setup {
void layout() {
    display.clearBuffer();
    ssd.fillRect(0, 0, 127, 8, __WHITE__);
    // print key
    display.add("MODE :", 5, 6, __BLACK__, 1);
    display.add("SSID :", 5, 17, __WHITE__, 1);
    display.add("IP   :", 5, 25, __WHITE__, 1);
    display.render();
}
void update_mode(const char *text) {
    String printValue = text;
    printValue.toUpperCase();
    display.add(printValue.c_str(), 35, 6, __BLACK__, 1);
    display.render();
}
void update_ssid() {
    String printString = WIFI_SSID;
    printString.toUpperCase();
    display.add(printString.c_str(), 35, 17, __WHITE__, 1);
    display.render();
}
void update_ip(const char *ip) {
    display.add(ip, 35, 25, __WHITE__, 1);
    display.render();
}
}; // namespace render_setup
// ========================================================================== //
// [prototype] normal mode
void normal_mode_setup();
void normal_mode_service();
stamp_t nms_i = 1000;
stamp_t nms_ts;
stamp_t play_i = 0;
stamp_t play_ts;
bool muted = false;
bool shown = false;
namespace render_normal {
void layout() {
    display.clearBuffer();
    display.add("IP:", 4, 17, __WHITE__, 1);
    display.add("NEXT:", 4, 27, __WHITE__, 1);
    display.render();
}
void update_ip(const char *ip) {
    // clear feild
    ssd.fillRect(31, 13, 127, 5, __BLACK__);
    // add
    display.add(ip, 31, 17, __WHITE__, 1);
    display.render();
}
void update_date(const char *date) {
    ssd.fillRect(4, 3, 127, 5, __BLACK__);
    String printValue = date;
    printValue.toUpperCase();
    display.add(printValue.c_str(), 4, 7, __WHITE__, 1);
    display.render();
}
void update_time(const char *time) {
    ssd.fillRect(90, 3, 127, 5, __BLACK__);
    display.add(time, 90, 7, __WHITE__, 1);
    display.render();
}
void update_next(const char *name) {
    ssd.fillRect(31, 23, 127, 5, __BLACK__);
    String printValue = name;
    printValue.toUpperCase();
    display.add(printValue.c_str(), 31, 27, __WHITE__, 1);
    display.render();
}
}; // namespace render_normal
// ========================================================================== //
// [prototype] debug mode
void debug_mode_setup();
void debug_mode_service();
stamp_t dms_i = 1000;
stamp_t dms_ts;
bool played = false;
namespace render_debug {
void layout() {
    display.clearBuffer();
    ssd.fillRect(0, 0, 127, 8, __WHITE__);
    display.add("DEBUG-PLAYING", 31, 6, __BLACK__, 1);
    display.add("NOW : ", 2, 16, __WHITE__, 1);
    display.add("STA : ", 2, 25, __WHITE__, 1);
    display.render();
}
void update_index(uint8_t index) {
    String printValue = tasks[index].name;
    printValue.toUpperCase();
    // clear field
    ssd.fillRect(26, 12, 127, 5, __BLACK__);
    // add
    display.add(printValue.c_str(), 26, 16, __WHITE__, 1);
    display.render();
}
void update_status(bool played) {
    ssd.fillRect(26, 21, 127, 5, __BLACK__);
    display.add((played) ? "PLAY" : "PAUSE", 26, 25, __WHITE__, 1);
    display.render();
}
}; // namespace render_debug
// ========================================================================== //
// [prototype] ota
void ota_setup();
void ota_service();
// ========================================================================== //
// [prototype] helper
void clock_update();
Time get_current_time();
void plog_task_table();
void stop_task();
// ========================================================================== //
// [definitions] normal mode
void normal_mode_setup() {
    // clear screen for screen saving
    display.clearBuffer();
    display.render();
    plog("\n* normal-mode setup");
    clock_update();
    plog.use("\n* current date(%i) time(%i)")
        .print(rtc.getTime(__DATE_F__).c_str(), green)
        .print(rtc.getTime(__TIME_F__).c_str(), green);
    // find next task
    Time current = get_current_time();
    if (current >= tasks[0].time && current < tasks[taskLen - 1].time) {
        for (size_t i = 0; i < taskLen; i++) {
            if (current < tasks[i].time) {
                taskIndex = i;
                break;
            }
        }
    }
}
void normal_mode_service() {
    /* core service */
    //=> start-playing service
    if (millis() - nms_ts >= nms_i) {
        nms_ts = millis();

        // update time on screen
        if (shown) {
            render_normal::update_date(rtc.getTime(__DATE_F__).c_str());
            render_normal::update_time(rtc.getTime(__TIME_F__).c_str());
        }

        Time current = get_current_time();
        plog.use("\n* rtc (%i:%i:%i) => (%i)")
            .print(current.hr, green)
            .print(current.min, green)
            .print(current.sec, green)
            .print(current.toSec(), blue);
        plog_task_table();
        // check to play task
        bool insideWorkTime = (current >= tasks[0].time && current <= tasks[taskLen - 1].time);
        if (insideWorkTime && current >= tasks[taskIndex].time) {
            if (!muted) {
                // play task
                mp3.play(tasks[taskIndex].index);
                play_ts = millis();
                play_i = (tasks[taskIndex].dura_sec * 1000);
            }
            // update index
            taskIndex = (taskIndex + 1) % taskLen;
        }
    }
    //=> stop-playing service
    if (millis() - play_ts >= play_i && play_i != 0) {
        stop_task();
    }

    /* side service */
    //=> manual stop
    if (topButton.read() == LOW) { // pushed top button
        stop_task();
    }
    //=> mute
    if (midButton.read() == LOW) { // pushed mid button
        // mute
        muted = !muted;
        if (muted) {
            errorLed.on();
        } else {
            errorLed.off();
        }
        stop_task();
    }
    //=> display infomation
    if (botButton.read() == LOW) { // pushed bottom button
        if (shown) {
            //=> un show
            display.clearBuffer();
            display.render();
            shown = false;
        } else {
            //=> show
            render_normal::layout();
            render_normal::update_date(rtc.getTime(__DATE_F__).c_str());
            render_normal::update_time(rtc.getTime(__TIME_F__).c_str());
            render_normal::update_ip(WiFi.localIP().toString().c_str());
            render_normal::update_next(tasks[taskIndex].name);
            shown = true;
        }
        // debounce
        delay(500);
    }
}

// ========================================================================== //
// [definitions] debug mode
void debug_mode_setup() {
    plog("\n* debug-mode setup");
    render_debug::layout();
    render_debug::update_index(taskIndex);
    render_debug::update_status(played);
}
void debug_mode_service() {
    delay(100);
    if (topButton.read() == LOW) { // play and pause
        if (played) {
            mp3.stop();
            play_i = 0;
            played = false;
        } else {
            mp3.play(tasks[taskIndex].index);
            play_ts = millis();
            play_i = tasks[taskIndex].dura_sec * 1000;
            played = true;
        }
        render_debug::update_status(played);
    }
    if (midButton.read() == LOW) { // next
        taskIndex = (taskIndex + 1) % taskLen;
        mp3.stop();
        play_i = 0;
        played = false;
        render_debug::update_status(played);
        render_debug::update_index(taskIndex);
    }
    if (botButton.read() == LOW) { // prev
        if (taskIndex == 0) {
            taskIndex = taskLen - 1;
        } else {
            taskIndex--;
        }
        mp3.stop();
        play_i = 0;
        played = false;
        render_debug::update_status(played);
        render_debug::update_index(taskIndex);
    }
    if (millis() - play_ts >= play_i && play_i != 0) {
        mp3.stop();
        play_i = 0;
        played = false;
        render_debug::update_status(played);
    }
}

// ========================================================================== //
// [definitions] ota
void ota_setup() {
    ArduinoOTA.setHostname("SchoolAlarmDevice");
    ArduinoOTA.setPasswordHash("bb0ea5d40d5897a0fc48aac9736ede19");
    ArduinoOTA.onStart([]() {
        esp_task_wdt_deinit();
        display.clearBuffer();
        display.add("! NEW FIRMWARE OTA", 0, 5, __WHITE__, 1);
        display.render();
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else { // U_FS
            type = "filesystem";
        }
        // NOTE: if updating FS this would be the place to unmount FS using FS.end()
        Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            Serial.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
            Serial.println("End Failed");
        }
    });
    ArduinoOTA.begin();
}
void ota_service() {
    ArduinoOTA.handle();
}

// ========================================================================== //
// [definitions] helper
void clock_update() {
    plog("\n* updating clock");
    HTTPClient http;
    // fetch
    http.begin(TIME_SERVER);
    int resCode;
    for (size_t i = 0; i < TIME_SERVER_ATTEMP; i++) {
        resCode = http.GET();
        delay(1000);
        if (resCode == 200) {
            break;
        }
    }
    StaticJsonDocument<512> jsonDoc;
    deserializeJson(jsonDoc, http.getString());
    rtc.setTime(
        jsonDoc["seconds"],
        jsonDoc["minute"],
        jsonDoc["hour"],
        jsonDoc["day"],
        jsonDoc["month"],
        jsonDoc["year"]);
}
Time get_current_time() {
    return {rtc.getHour(true), rtc.getMinute(), rtc.getSecond()};
}
void plog_task_table() {
    plog("\n* tasks table");
    for (size_t i = 0; i < taskLen; i++) {
        SoundTask t = tasks[i];
        if (taskIndex == i) {
            plog("\n-->", red);
        } else {
            plog("\n   ");
        }
        plog.use("[ @ %i:%i:%i play-index %i for %i sec ]--- %i")
            .print(t.time.hr, green, 2)(t.time.min, green, 2)(t.time.sec, green, 2)
            .print(t.index, green, 2)
            .print(t.dura_sec, green, 3)
            .print(t.name, green);
    }
}
void stop_task() {
    // stop current playing
    mp3.stop();
    // set parameter
    play_i = 0;
}

// ========================================================================== //
// run time
void setup() {
    Serial.begin(115200);
    mp3.begin(9600);
    mp3.sendCommand(CMD_SEL_DEV, 0, 2);
    mp3.setVol(30);
    errorLed.setup(LOW);
    topButton.setup();
    midButton.setup();
    botButton.setup();
    ssd.begin(SSD1306_SWITCHCAPVCC, 0x3c);
    ssd.setFont(&Font5x5Fixed);
    ssd.setTextWrap(false);
    // mode selection
    render_setup::layout();
    uint8_t modeReaded = topButton.read();
    if (modeReaded == LOW) { // pushed => debug-mode
        plog("\n* debug-mode", magenta);
        render_setup::update_mode("debug");
        mode_setup = debug_mode_setup;
        mode_service = debug_mode_service;
    } else {
        plog("\n* normal-mode", magenta);
        render_setup::update_mode("normal");
        mode_setup = normal_mode_setup;
        mode_service = normal_mode_service;
    }
    // connect wifi
    render_setup::update_ssid();
    plog.use("\n* connecting {%i, %i} ")(WIFI_SSID, green)(WIFI_PSK, green);
    WiFi.disconnect();
    while (WiFi.status() == WL_CONNECTED) {
        delay(100);
    }
    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PSK);
    if (WiFi.waitForConnectResult() != WL_CONNECTED) {
        plog("\n! can't connnect wifi", red);
        errorLed.on();
        delay(5000);
        ESP.restart();
    }
    render_setup::update_ip(WiFi.localIP().toString().c_str());
    plog.use("\n* ip -> %i")(WiFi.localIP().toString().c_str(), green);
    delay(2000);
    // ota setup
    ota_setup();
    // mode setup
    mode_setup();
    // set watchdog
    esp_task_wdt_init(LOOP_TIMEOUT, true);
    esp_task_wdt_add(NULL);
}
void loop() {
    // mode service
    mode_service();
    // ota service
    ota_service();
    // reset watchdog
    esp_task_wdt_reset();
}

//!------------------------------------------------------------------------------ [end]