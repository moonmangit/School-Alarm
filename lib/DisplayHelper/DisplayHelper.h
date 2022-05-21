#ifndef __DISPLAYHELPER__H__
#define __DISPLAYHELPER__H__

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#ifndef COLOR_WHITE
#define COLOR_WHITE SSD1306_WHITE
#endif

#ifndef COLOR_BLACK
#define COLOR_BLACK SSD1306_BLACK
#endif

struct DisplayPosition {
    int16_t x, y;
};

class DisplayHelper {
private:
    Adafruit_SSD1306 *ssd;

public:
    DisplayHelper(Adafruit_SSD1306 *ssd) {
        this->ssd = ssd;
    }
    ~DisplayHelper() {}

    void render();
    DisplayHelper &clear();

    /* text render helper */
    DisplayHelper &prop(uint8_t color, uint8_t size);
    DisplayHelper &at(uint8_t x, uint8_t y);
    DisplayHelper &at(DisplayPosition pos);
    DisplayHelper &add(const char *txt);
    DisplayHelper &add(char c);
    DisplayHelper &add(int n);
    DisplayHelper &operator()(char c);
    DisplayHelper &operator()(int n);
    DisplayHelper &addNewLine(const char *txt);

    /* line render helper  */
    DisplayHelper &vline(uint8_t x);
    DisplayHelper &hline(uint8_t y);

    /* others */
    DisplayPosition getPos();
};

#endif //!__DISPLAYHELPER__H__