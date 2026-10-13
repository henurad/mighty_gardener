#include "pigpio.h"

#include <chrono>
#include <thread>

int gpioInitialise(void) {
    return 0;
}

void gpioTerminate(void) {
}

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
    using namespace std::chrono;
    auto now = steady_clock::now().time_since_epoch();
    return static_cast<uint32_t>(duration_cast<microseconds>(now).count());
}
