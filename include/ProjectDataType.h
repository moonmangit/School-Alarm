#include <Arduino.h>

typedef unsigned long stamp_t;

struct Time {
    int hr, min, sec;
    uint32_t toSec() { return (uint32_t(hr) * 3600) + uint32_t(min) * 60 + uint32_t(sec); }
    bool operator==(Time t) { return (this->toSec() == t.toSec()); }
    bool operator!=(Time t) { return (this->toSec() != t.toSec()); }
    bool operator>(Time t) { return (this->toSec() > t.toSec()); }
    bool operator>=(Time t) { return (this->toSec() >= t.toSec()); }
    bool operator<(Time t) { return (this->toSec() < t.toSec()); }
    bool operator<=(Time t) { return (this->toSec() <= t.toSec()); }
};

struct SoundTask {
    Time time;
    uint8_t index;
    stamp_t duration;
    const char *name;
};