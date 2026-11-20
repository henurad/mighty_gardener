#include "hardware/relay.h"

#include <iostream>
#include <stdexcept>

#ifdef HAVE_GPIOD
#include <gpiod.h>
#endif

Relay::Relay(int gpio_pin, bool active_low)
    : line_(gpio_pin, GpioLine::Direction::kOutput, active_low),
      active_low_(active_low) {
  if (!line_.Open()) {
    std::cerr << "Relay GPIO line " << gpio_pin
              << " could not be initialized.\n";
  }
}

Relay::~Relay() = default;

void Relay::TurnOn() {
  line_.SetValue(!active_low_);
}

void Relay::TurnOff() {
  line_.SetValue(active_low_);
}
