#include <relay.h>

#include <iostream>
#include <stdexcept>

#ifdef HAVE_GPIOD
#include <gpiod.h>
#endif

Relay::Relay(int gpioPin, bool active_low = false)
    : pin(gpioPin), activeLow(active_low) {
#ifdef HAVE_GPIOD
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
#else
    (void)gpioPin;
    (void)active_low;
    std::cerr << "GPIO support is unavailable; relay operations are disabled." << std::endl;
#endif
}

Relay::~Relay() {
#ifdef HAVE_GPIOD
    if (line) gpiod_line_release(line);
    if (chip) gpiod_chip_close(chip);
#endif
}

void Relay::turnOn() {
#ifdef HAVE_GPIOD
    int value = activeLow ? 0 : 1;
    gpiod_line_set_value(line, value);
#else
    (void)0;
#endif
}

void Relay::turnOff() {
#ifdef HAVE_GPIOD
    int value = activeLow ? 1 : 0;
    gpiod_line_set_value(line, value);
#else
    (void)0;
#endif
}
