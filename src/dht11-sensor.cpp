#include <dht11-sensor.h>

#include <pigpio.h>
#include <stdint.h>
#include <unistd.h>
#include <iostream>

DHT11Sensor::DHT11Sensor(int gpioPin)
    : gpio(gpioPin), initialized(false), monitoring(false) {
    for (int i = 0; i < 5; i++) {
        data[i] = 0;
    }
}

DHT11Sensor::~DHT11Sensor() {
    stopMonitoring();
    join();
    shutdown();
}

bool DHT11Sensor::initialize() {
    if (initialized) {
        return true;
    }

    if (gpioInitialise() < 0) {
        std::cerr << "pigpio initialization failed\n";
        return false;
    }

    initialized = true;
    return true;
}

void DHT11Sensor::shutdown() {
    if (initialized) {
        gpioTerminate();
        initialized = false;
    }
}

void DHT11Sensor::startMonitoring(int intervalSeconds) {
    if (monitoring.load()) {
        return;
    }

    if (!initialize()) {
        return;
    }

    monitoring.store(true);
    dht11_thread = std::thread([this, intervalSeconds]() {
        this->monitor(intervalSeconds);
    });
}

void DHT11Sensor::stopMonitoring() {
    monitoring.store(false);
}

void DHT11Sensor::join() {
    if (dht11_thread.joinable()) {
        dht11_thread.join();
    }
}

void DHT11Sensor::monitor(int intervalSeconds) {
    while (monitoring.load()) {
        float temperature = 0.0f;
        float humidity = 0.0f;

        if (read(temperature, humidity)) {
            std::cout << "Temperature: "
                      << temperature
                      << " °C | Humidity: "
                      << humidity
                      << " %"
                      << std::endl;
        } else {
            std::cout << "Failed to read DHT11" << std::endl;
        }

        sleep(intervalSeconds);
    }
}

bool DHT11Sensor::read(float &temperature, float &humidity) {
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
