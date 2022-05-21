#ifndef __BUTTON__H__
#define __BUTTON__H__

#include <Arduino.h>

class Button {
private:
    uint8_t pin, mode;
    uint8_t recent;

public:
    Button(uint8_t pin, uint8_t mode = INPUT_PULLUP) {
        this->pin = pin;
        this->mode = mode;
    }
    ~Button() {}
    void setup() {
        pinMode(pin, mode);
    }
    uint8_t read() {
        recent = digitalRead(pin);
        return recent;
    }
    uint8_t getRecent() {
        return recent;
    }
};

#endif //!__BUTTON__H__