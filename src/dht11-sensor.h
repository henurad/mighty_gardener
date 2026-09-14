#ifndef DHT11_SENSOR_H
#define DHT11_SENSOR_H

#include <stdint.h>

class DHT11Sensor {
private:
    int gpio;
    uint8_t data[5];

public:
    DHT11Sensor(int gpioPin);
    bool read(float &temperature, float &humidity);
};

#endif