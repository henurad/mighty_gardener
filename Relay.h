#include <gpiod.h>
#include <iostream>
#include <stdexcept>

class Relay {
private:
    gpiod_chip* chip;
    gpiod_line* line;
    int pin;
    bool activeLow;

public:
    Relay(int gpioPin, bool active_low = false)
        : pin(gpioPin), activeLow(active_low)
    {
        chip = gpiod_chip_open_by_name("gpiochip0");
        if (!chip) {
            throw std::runtime_error("Failed to open gpiochip0");
        }

        line = gpiod_chip_get_line(chip, pin);
        if (!line) {
            throw std::runtime_error("Failed to get GPIO line");
        }

        if (gpiod_line_request_output(line, "relay", activeLow ? 1 : 0) < 0) {
            throw std::runtime_error("Failed to request line as output");
        }
    }

    void turnOn() {
        int value = activeLow ? 0 : 1;
        gpiod_line_set_value(line, value);
    }

    void turnOff() {
        int value = activeLow ? 1 : 0;
        gpiod_line_set_value(line, value);
    }

    ~Relay() {
        if (line) gpiod_line_release(line);
        if (chip) gpiod_chip_close(chip);
    }
};