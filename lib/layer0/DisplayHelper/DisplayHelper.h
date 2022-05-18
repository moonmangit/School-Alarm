#ifndef __DISPLAYHELPER__H__
#define __DISPLAYHELPER__H__

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#ifndef __WHITE__
#define __WHITE__ SSD1306_WHITE
#endif

#ifndef __BLACK__
#define __BLACK__ SSD1306_BLACK
#endif

class DisplayHelper {
private:
    Adafruit_SSD1306 *ssd;

public:
    DisplayHelper(Adafruit_SSD1306 *ssd) {
        this->ssd = ssd;
    }
    ~DisplayHelper() {}
    void render();
    void clearBuffer();
    void add(const char *text, uint8_t x, uint8_t y, uint8_t color, uint8_t textSize = 1);
};

#endif //!__DISPLAYHELPER__H__