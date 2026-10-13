#ifndef DHT11_SENSOR_H
#define DHT11_SENSOR_H

#include <atomic>
#include <stdint.h>
#include <thread>

class DHT11Sensor {
private:
    int gpio;
    uint8_t data[5];
    bool initialized;
    std::atomic<bool> monitoring;
    std::thread dht11_thread;

public:
    explicit DHT11Sensor(int gpioPin);
    ~DHT11Sensor();

    bool initialize();
    void shutdown();
    void startMonitoring(int intervalSeconds = 5);
    void stopMonitoring();
    void join();
    void monitor(int intervalSeconds = 5);
    bool read(float &temperature, float &humidity);
};

#endif