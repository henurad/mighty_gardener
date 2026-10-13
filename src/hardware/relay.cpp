#include "hardware/relay.h"

#include <iostream>
#include <stdexcept>

#ifdef HAVE_GPIOD
#include <gpiod.h>
#endif

Relay::Relay(int gpio_pin, bool active_low)
    : pin_(gpio_pin), active_low_(active_low) {
#ifdef HAVE_GPIOD
  chip_ = gpiod_chip_open_by_name("gpiochip0");
  if (chip_ == nullptr) {
    throw std::runtime_error("Failed to open gpiochip0");
  }

  line_ = gpiod_chip_get_line(chip_, pin_);
  if (line_ == nullptr) {
    throw std::runtime_error("Failed to get GPIO line");
  }

  if (gpiod_line_request_output(line_, "relay", active_low_ ? 1 : 0) < 0) {
    throw std::runtime_error("Failed to request line as output");
  }
#else
  (void)gpio_pin;
  (void)active_low;
  std::cerr << "GPIO support is unavailable; relay operations are disabled."
            << std::endl;
#endif
}

Relay::~Relay() {
#ifdef HAVE_GPIOD
  if (line_ != nullptr) {
    gpiod_line_release(line_);
  }
  if (chip_ != nullptr) {
    gpiod_chip_close(chip_);
  }
#endif
}

void Relay::TurnOn() {
#ifdef HAVE_GPIOD
  const int value = active_low_ ? 0 : 1;
  gpiod_line_set_value(line_, value);
#endif
}

void Relay::TurnOff() {
#ifdef HAVE_GPIOD
  const int value = active_low_ ? 1 : 0;
  gpiod_line_set_value(line_, value);
#endif
}
