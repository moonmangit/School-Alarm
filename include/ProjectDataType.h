#ifndef __PROJECTDATATYPE__H__
#define __PROJECTDATATYPE__H__

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
    uint16_t index, dura_sec;
    const char *name;
};

#endif //!__PROJECTDATATYPE__H__