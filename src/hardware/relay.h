#ifndef HARDWARE_RELAY_H_
#define HARDWARE_RELAY_H_

#include "hardware/gpio-line.h"

class Relay {
 public:
  Relay(int gpio_pin, bool active_low);
  ~Relay();

  Relay(const Relay&) = delete;
  Relay& operator=(const Relay&) = delete;

  void TurnOn();
  void TurnOff();

 private:
  GpioLine line_;
  bool active_low_;
};

#endif  // HARDWARE_RELAY_H_
