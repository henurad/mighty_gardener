#ifndef RELAY_H_
#define RELAY_H_

#ifdef HAVE_GPIOD
#include <gpiod.h>
#endif

class Relay {
private:
#ifdef HAVE_GPIOD
    gpiod_chip* chip;
    gpiod_line* line;
#else
    void* chip = nullptr;
    void* line = nullptr;
#endif
    int pin;
    bool activeLow;

public:
    Relay(int gpioPin, bool active_low);
    void turnOn();
    void turnOff();
    ~Relay();
};

#endif  // RELAY_H_
