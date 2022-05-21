#ifndef __LED__H__
#define __LED__H__

#include <Arduino.h>

class Led {
private:
    uint8_t pin, mode;
    uint8_t recent;
    bool activeHigh;

public:
    Led(uint8_t pin, bool activeHigh = true, uint8_t mode = OUTPUT) {
        this->pin = pin;
        this->activeHigh = activeHigh;
        this->mode = mode;
    }
    ~Led() {}
    void setup(uint8_t defaultValue = LOW) {
        pinMode(pin, mode);
        digitalWrite(pin, defaultValue);
    }
    void on() {
        recent = (activeHigh ? 1 : 0);
        digitalWrite(pin, recent);
    }
    void off() {
        recent = (activeHigh ? 0 : 1);
        digitalWrite(pin, recent);
    }
    void toggle() {
        recent = (recent + 1) % 2;
        digitalWrite(pin, recent);
    }
};

#endif //!__LED__H__