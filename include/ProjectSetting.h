#ifndef __PROJECTSETTING__H__
#define __PROJECTSETTING__H__

//------------------------------------------------------------------------------ [connection]
#define WIFI_SSID "wifi-Seagame"
#define WIFI_PSK "123456789"
#define TIME_SERVER "https://www.timeapi.io/api/Time/current/zone?timeZone=Asia/Bangkok"
#define TIME_SERVER_ATTEMP 5
//------------------------------------------------------------------------------ [pins]
#define PIN_BUTTON_1 32
#define PIN_BUTTON_2 33
#define PIN_BUTTON_3 25
#define PIN_LED_1 26
#define PIN_MP3_TX 16
#define PIN_MP3_RX 17
#define SSD_ADDR 0x3c
//------------------------------------------------------------------------------ [watchdog]
#define LOOP_TIMEOUT 3

#endif //!__PROJECTSETTING__H__