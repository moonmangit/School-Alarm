#include "DisplayHelper.h"

void DisplayHelper::render() {
    ssd->display();
}
DisplayHelper &DisplayHelper::clear() {
    ssd->clearDisplay();
    return *this;
}

/* text render helper */
DisplayHelper &DisplayHelper::prop(uint8_t color, uint8_t size) {
    ssd->setTextColor(color);
    ssd->setTextSize(size);
    return *this;
}
DisplayHelper &DisplayHelper::at(uint8_t x, uint8_t y) {
    ssd->setCursor(x, y);
    return *this;
}
DisplayHelper &DisplayHelper::at(DisplayPosition pos) {
    ssd->setCursor(pos.x, pos.y);
    return *this;
}
DisplayHelper &DisplayHelper::add(const char *txt) {
    ssd->print(txt);
    return *this;
}
DisplayHelper &DisplayHelper::add(char c) {
    ssd->print(c);
    return *this;
}
DisplayHelper &DisplayHelper::add(int n) {
    ssd->print(n);
    return *this;
}
DisplayHelper &DisplayHelper::operator()(char c) {
    ssd->print(c);
    return *this;
}
DisplayHelper &DisplayHelper::operator()(int n) {
    ssd->print(n);
    return *this;
}
DisplayHelper &DisplayHelper::addNewLine(const char *txt) {
    ssd->println();
    ssd->print(txt);
    return *this;
}

/* line render helper  */
DisplayHelper &DisplayHelper::vline(uint8_t x) {
    ssd->drawFastVLine(x, 0, ssd->height(), COLOR_WHITE);
    return *this;
}
DisplayHelper &DisplayHelper::hline(uint8_t y) {
    ssd->drawFastHLine(0, y, ssd->width(), COLOR_WHITE);
    return *this;
}

/* others */
DisplayPosition DisplayHelper::getPos() {
    return {ssd->getCursorX(), ssd->getCursorY()};
}