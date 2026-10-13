#include "hardware/pigpio.h"

#include <chrono>
#include <thread>

int gpioInitialise(void) { return 0; }

void gpioTerminate(void) {}

void gpioSetMode(unsigned int gpio, unsigned int mode) {
  (void)gpio;
  (void)mode;
}

void gpioWrite(unsigned int gpio, unsigned int level) {
  (void)gpio;
  (void)level;
}

void gpioDelay(unsigned int microseconds) {
  if (microseconds > 0) {
    std::this_thread::sleep_for(std::chrono::microseconds(microseconds));
  }
}

int gpioRead(unsigned int gpio) {
  (void)gpio;
  return 1;
}

uint32_t gpioTick(void) {
  const auto now = std::chrono::steady_clock::now().time_since_epoch();
  return static_cast<uint32_t>(
      std::chrono::duration_cast<std::chrono::microseconds>(now).count());
}
