#ifndef HARDWARE_RELAY_H_
#define HARDWARE_RELAY_H_

#ifdef HAVE_GPIOD
#include <gpiod.h>
#endif

class Relay {
 public:
  Relay(int gpio_pin, bool active_low);
  ~Relay();

  Relay(const Relay&) = delete;
  Relay& operator=(const Relay&) = delete;

  void TurnOn();
  void TurnOff();

 private:
#ifdef HAVE_GPIOD
  gpiod_chip* chip_ = nullptr;
  gpiod_line* line_ = nullptr;
#else
  void* chip_ = nullptr;
  void* line_ = nullptr;
#endif
  int pin_;
  bool active_low_;
};

#endif  // HARDWARE_RELAY_H_
