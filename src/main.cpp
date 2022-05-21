#include "ProjectSetting.h"

namespace DEVICES {
// LED
Led errorLed(PIN_LED_ERR);
// BUTTON
Button topButton(PIN_BUTTON_TOP);
Button midButton(PIN_BUTTON_MID);
Button botButton(PIN_BUTTON_BOT);
// MP3
SerialMP3Player mp3;
// SSD1306
Adafruit_SSD1306 ssd(SSD_WIDTH, SSD_HEIGHT, &Wire, -1, 400000UL, 100000UL);
DisplayHelper display(&ssd);
void setup() {
    Serial.begin(MONITOR_SPEED);
    // Led
    errorLed.setup();
    // Buttons
    topButton.setup();
    midButton.setup();
    botButton.setup();
    // MP3
    mp3.begin(MP3_SPEED);
    mp3.sendCommand(CMD_SEL_DEV, 0, 2);
    mp3.setVol(MP3_VOL);
    // SSD1306
    ssd.begin(SSD1306_SWITCHCAPVCC, SSD_ADDR);
    ssd.setTextWrap(false);
    display.clear().render();
}
} // namespace DEVICES

namespace PLAYER {
int tasksIndex = 0;
bool muted = false;
bool displayed = true;
stamp_t playTimeStamp, playIntervals;
void play() {
    // send play command
    DEVICES::mp3.play(tasks[tasksIndex].index);
    // update play variables
    playTimeStamp = millis();
    playIntervals = (tasks[tasksIndex].duration);
}
void stop() {
    // send stop command
    DEVICES::mp3.stop();
    // update play variable
    playIntervals = 0;
}
void handle_check_stop() {
    if (millis() - playTimeStamp >= playIntervals && playIntervals != 0) {
        stop();
    }
}
}; // namespace PLAYER

namespace CLOCK {
ESP32Time rtc;
Time getTime() {
    return {rtc.getHour(true), rtc.getMinute(), rtc.getSecond()};
}
}; // namespace CLOCK

namespace AUTO_MODE {
stamp_t handleStamp, handlePeriod = 1000;
DisplayPosition timePos, nextPos, nextTimePos;
void render_static() {
    DEVICES::display
        .clear()
        .prop(COLOR_WHITE, 1)
        .at(0, 0)
        .add("AUTO @ ");
    timePos = DEVICES::display.getPos();
    DEVICES::display
        .addNewLine("IP: ")
        .add(WiFi.localIP().toString().c_str())
        .addNewLine("NXT: ");
    nextPos = DEVICES::display.getPos();
    DEVICES::display
        .addNewLine("NXT@: ")
        .render();
    nextTimePos = DEVICES::display.getPos();
}
void render_time() {
    DEVICES::ssd.fillRect(timePos.x, timePos.y, 128, 8, COLOR_BLACK);
    DEVICES::display
        .at(timePos)
        .add(CLOCK::rtc.getTime(__TIME_F__).c_str())
        .render();
}
void render_next() {
    // render next-name
    DEVICES::ssd.fillRect(nextPos.x, nextPos.y, 128, 8, COLOR_BLACK);
    DEVICES::display
        .at(nextPos)
        .add(tasks[PLAYER::tasksIndex].name)
        .render();
    // render next-time
    DEVICES::ssd.fillRect(nextTimePos.x, nextTimePos.y, 128, 8, COLOR_BLACK);
    Time &t = tasks[PLAYER::tasksIndex].time;
    DEVICES::display
        .at(nextTimePos)
        .add(t.hr)(':')(t.min)(':')(t.sec)
        .render();
}
void handle_check_buttons() {
    if (DEVICES::topButton.read() == LOW) { // top(stop) button
        delay(250);
        PLAYER::stop();
    }
    if (DEVICES::midButton.read() == LOW) { // mid(mute/unmute) button
        delay(250);
        PLAYER::muted = !PLAYER::muted;
        if (PLAYER::muted) {
            DEVICES::errorLed.on();
        } else {
            DEVICES::errorLed.off();
        }
        PLAYER::stop();
    }
    if (DEVICES::botButton.read() == LOW) { // bot(show/unshow) button
        delay(250);
        PLAYER::displayed = !PLAYER::displayed;
        if (PLAYER::displayed) {
            render_static();
            render_time();
            render_next();
        } else {
            DEVICES::display.clear().render();
        }
    }
}
void handle_check_play(Time &current) {
    // print value
    Serial.print("\033[2J\033[H");
    SoundTask &task = tasks[PLAYER::tasksIndex];
    plog.use("\n* auto <%i> [%i:%i:%i] : %i @ [%i:%i]", yellow)
        .print(B2OPTS(PLAYER::muted, "muted", "unmuted"), (PLAYER::muted) ? red : green)
        .print(current.hr, blue)(current.min, blue)(current.sec, blue)
        .print(task.name, blue)(task.time.hr, blue)(task.time.min, blue);
    // check to play
    bool insideWorkTime = (current >= tasks[0].time && current <= tasks[tasksLength - 1].time);
    if (insideWorkTime && current >= tasks[PLAYER::tasksIndex].time) {
        if (!PLAYER::muted) {
            PLAYER::play();
        }
        PLAYER::tasksIndex = (PLAYER::tasksIndex + 1) % tasksLength;
        // update next-task on screen
        if (PLAYER::displayed) {
            render_next();
        }
    }
}
void setup() {
    render_static();
    // update clock
    plog.use("\n* update clock [%i] : %i", yellow)(TIME_SERVER, blue);
    HTTPClient http;
    http.begin(TIME_SERVER);
    for (size_t i = 0; i < TIME_SERVER_ATTEMP; i++) {
        if (http.GET() == 200) {
            break;
        }
    }
    StaticJsonDocument<512> doc;
    deserializeJson(doc, http.getString());
    CLOCK::rtc.setTime(
        doc["seconds"],
        doc["minute"],
        doc["hour"],
        doc["day"],
        doc["month"],
        doc["year"]);
    plog(CLOCK::rtc.getTime(__TIME_F__).c_str(), blue);
    render_time();
    // find next task
    plog.use("\n* next sound-task : %i @ %i:%i", yellow);
    Time current = CLOCK::getTime();
    if (current >= tasks[0].time && current < tasks[tasksLength - 1].time) {
        for (size_t i = 0; i < tasksLength; i++) {
            if (current < tasks[i].time) {
                PLAYER::tasksIndex = i;
                break;
            }
        }
    }
    SoundTask &task = tasks[PLAYER::tasksIndex];
    plog(task.name, blue)(task.time.hr, blue)(task.time.min, blue);
    render_next();
}
void handle() {
    if (millis() - handleStamp >= handlePeriod) {
        handleStamp = millis();
        Time time = CLOCK::getTime();
        handle_check_play(time);
        // update time on screen
        if (PLAYER::displayed) {
            render_time();
        }
    }
    PLAYER::handle_check_stop();
    handle_check_buttons();
}
}; // namespace AUTO_MODE

namespace MANUAL_MODE {
DisplayPosition nextPos, statPos;
void render_static() {
    DEVICES::display
        .clear()
        .prop(COLOR_WHITE, 1)
        .at(0, 0)
        .add("MANUAL")
        .addNewLine("IP: ")
        .add(WiFi.localIP().toString().c_str())
        .addNewLine("CUR: ");
    nextPos = DEVICES::display.getPos();
    DEVICES::display.addNewLine("STA: ");
    statPos = DEVICES::display.getPos();
    DEVICES::display.render();
}
void render_next() {
    DEVICES::ssd.fillRect(nextPos.x, nextPos.y, 128, 8, COLOR_BLACK);
    DEVICES::display
        .at(nextPos)
        .add(tasks[PLAYER::tasksIndex].name)
        .render();
}
void render_play_stat() {
    DEVICES::ssd.fillRect(statPos.x, statPos.y, 128, 8, COLOR_BLACK);
    DEVICES::display
        .at(statPos)
        .add((PLAYER::playIntervals == 0) ? "STOP" : "PLAY")
        .render();
}
void plog_manual() {
    SoundTask &task = tasks[PLAYER::tasksIndex];
    plog.use("\n* manual <%i>: %i @ [%i:%i]")
        .print((PLAYER::playIntervals) ? "play" : "stop", (PLAYER::playIntervals) ? green : red)
        .print(task.name, blue)(task.time.hr, blue)(task.time.min, blue);
}
void handle_check_buttons() {
    if (DEVICES::topButton.read() == LOW) { // top(play/stop) button
        if (PLAYER::playIntervals != 0) {
            PLAYER::stop();
        } else {
            PLAYER::play();
        }
        render_play_stat();
        plog_manual();
        delay(250);
    }
    if (DEVICES::midButton.read() == LOW) { // mid(next) button
        PLAYER::tasksIndex = (PLAYER::tasksIndex + 1) % tasksLength;
        PLAYER::stop();
        render_next();
        render_play_stat();
        plog_manual();
        delay(250);
    }
    if (DEVICES::botButton.read() == LOW) { // bot(previous) button
        if (PLAYER::tasksIndex == 0) {
            PLAYER::tasksIndex = tasksLength - 1;
        } else {
            PLAYER::tasksIndex--;
        }
        render_next();
        render_play_stat();
        PLAYER::stop();
        plog_manual();
        delay(250);
    }
}
void setup() {
    PLAYER::tasksIndex = 0;
    PLAYER::playIntervals = 0;
    plog_manual();
    render_static();
    render_next();
    render_play_stat();
}
void handle() {
    handle_check_buttons();
    PLAYER::handle_check_stop();
}
}; // namespace MANUAL_MODE

void (*mode_setup)();
void (*mode_handle)();
void mode_select() {
    DEVICES::
        display
            .clear()
            .at(0, 0)
            .add("MODE: ")
            .render();
    plog.use("\n* mode-select : %i");
    uint8_t modeValue = DEVICES::topButton.read();
    if (modeValue == LOW) {
        DEVICES::display
            .at(DEVICES::display.getPos())
            .add("MANUAL-MODE")
            .render();
        plog("manual-mode", magenta);
        mode_setup = MANUAL_MODE::setup;
        mode_handle = MANUAL_MODE::handle;
    } else {
        DEVICES::display
            .at(DEVICES::display.getPos())
            .add("AUTO-MODE")
            .render();
        plog("auto-mode", magenta);
        mode_setup = AUTO_MODE::setup;
        mode_handle = AUTO_MODE::handle;
    }
}
void connect_wifi() {
    DEVICES::display
        .at(DEVICES::display.getPos())
        .addNewLine("SID: ")
        .add(WIFI_SSID)
        .addNewLine("IP: ")
        .render();
    plog.use("\n* wifi connect [%i] : %i")(WIFI_SSID, blue);
    WiFi.disconnect();
    while (WiFi.status() == WL_CONNECTED) {
        delay(1000);
    }
    WiFi.begin(WIFI_SSID, WIFI_PSK);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
    }
    if (WiFi.status() != WL_CONNECTED) {
        plog("can't connect", red);
        DEVICES::errorLed.on();
        while (1) {
            delay(250);
        }
    } else {
        plog(WiFi.localIP().toString().c_str(), green);
    }
    DEVICES::display
        .add(WiFi.localIP().toString().c_str())
        .render();
}
void ota_setup() {
    ArduinoOTA.setPasswordHash(OTA_AUTH_KEY);
    ArduinoOTA.onStart([]() {
        delay(1000);
        DEVICES::display.clear()
            .at(0, 0)
            .add("! FIRMWARE UPDATING ...")
            .render();
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
        esp_task_wdt_reset();
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
void ota_handle() {
    ArduinoOTA.handle();
}

//------------------------------------------------------------------------------ [run time]
void setup() {
    DEVICES::setup();
    DEVICES::display.clear()
        .at(0, 0)
        .prop(COLOR_WHITE, 1)
        .add("SCHOOL-ALARM-V2")
        .render();
    delay(1000);
    plog("\n* (1) : mode select", blue);
    mode_select();
    plog("\n* (2) : connect to wifi", blue);
    connect_wifi();
    plog("\n* (3) : ota setup", blue);
    ota_setup();
    plog("\n* (4) : mode setup", blue);
    mode_setup();
    // set watch dog
    esp_task_wdt_init(WDT_HANDLE_SEC, true);
    esp_task_wdt_add(NULL);
}

void loop() {
    ota_handle();
    mode_handle();
    // reset watch dog
    esp_task_wdt_reset();
}
