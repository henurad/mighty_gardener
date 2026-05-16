#ifndef DHT11_SENSOR_H
#define DHT11_SENSOR_H

#include <pigpio.h>
#include <stdint.h>
#include <unistd.h>
#include <iostream>

class DHT11Sensor {
private:
    int gpio;
    uint8_t data[5];

public:
    DHT11Sensor(int gpioPin) : gpio(gpioPin) {}

    bool read(float &temperature, float &humidity) {
        for (int i = 0; i < 5; i++)
            data[i] = 0;

        // Send start signal
        gpioSetMode(gpio, PI_OUTPUT);
        gpioWrite(gpio, 0);
        gpioDelay(18000); // 18 ms

        gpioWrite(gpio, 1);
        gpioDelay(40);

        gpioSetMode(gpio, PI_INPUT);

        // Wait for DHT response
        uint32_t timeout = 10000;

        while (gpioRead(gpio) == 1)
            if (--timeout == 0) return false;

        timeout = 10000;
        while (gpioRead(gpio) == 0)
            if (--timeout == 0) return false;

        timeout = 10000;
        while (gpioRead(gpio) == 1)
            if (--timeout == 0) return false;

        // Read 40 bits
        for (int i = 0; i < 40; i++) {

            // Wait for LOW
            timeout = 10000;
            while (gpioRead(gpio) == 0)
                if (--timeout == 0) return false;

            uint32_t startTick = gpioTick();

            // Wait for HIGH end
            timeout = 10000;
            while (gpioRead(gpio) == 1)
                if (--timeout == 0) return false;

            uint32_t duration = gpioTick() - startTick;

            data[i / 8] <<= 1;

            // >50us means bit 1
            if (duration > 50)
                data[i / 8] |= 1;
        }

        // Verify checksum
        uint8_t checksum =
            (data[0] + data[1] + data[2] + data[3]) & 0xFF;

        if (checksum != data[4])
            return false;

        humidity = data[0];
        temperature = data[2];

        return true;
    }
};

#endif