#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef PI_OUTPUT
#define PI_OUTPUT 1
#endif

#ifndef PI_INPUT
#define PI_INPUT 0
#endif

#ifndef PI_LOW
#define PI_LOW 0
#endif

#ifndef PI_HIGH
#define PI_HIGH 1
#endif

int gpioInitialise(void);
void gpioTerminate(void);
void gpioSetMode(unsigned int gpio, unsigned int mode);
void gpioWrite(unsigned int gpio, unsigned int level);
void gpioDelay(unsigned int microseconds);
int gpioRead(unsigned int gpio);
uint32_t gpioTick(void);

#ifdef __cplusplus
}
#endif
