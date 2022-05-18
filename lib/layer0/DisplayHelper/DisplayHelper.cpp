#include "DisplayHelper.h"

void DisplayHelper::render() {
    ssd->display();
}
void DisplayHelper::clearBuffer() {
    ssd->clearDisplay();
}
void DisplayHelper::add(const char *text, uint8_t x, uint8_t y, uint8_t color, uint8_t textSize) {
    ssd->setTextColor(color);
    ssd->setTextSize(textSize);
    ssd->setCursor(x, y);
    ssd->print(text);
}