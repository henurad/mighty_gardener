#include "sun-relay.h"

#include <chrono>
#include <ctime>

SunRelay::SunRelay(SerialPort& serial_port,
                 Relay& sun_light_relay,
                 Relay& heater_relay,
                 Relay& watering_relay)
    : serial_port_(serial_port),
      sun_light_relay_(sun_light_relay),
      heater_relay_(heater_relay),
      watering_relay_(watering_relay) {}

SunRelay::~SunRelay() {
    running_.store(false);
    Join();
}

void SunRelay::Start() {
    if (running_.load()) {
        return;
    }

    running_.store(true);
    worker_ = std::thread(&SunRelay::Run, this);
}

void SunRelay::Join() {
    if (worker_.joinable()) {
        worker_.join();
    }
}

void SunRelay::Run() {
    while (running_.load() && serial_port_.isOpen()) {
        std::time_t t = std::time(nullptr);
        std::tm local = *std::localtime(&t);
        int hour = local.tm_hour;
        int minute = local.tm_min;

        if (hour >= 18 && hour < 24) {
            sun_light_relay_.turnOn();
        } else {
            sun_light_relay_.turnOff();
        }

        if (hour >= 8 && hour <= 20) {
            heater_relay_.turnOff();
        } else {
            heater_relay_.turnOn();
        }

        if (hour == 12 && minute < 2) {
            watering_relay_.turnOn();
        } else {
            watering_relay_.turnOff();
        }

        std::this_thread::sleep_for(std::chrono::seconds(10));
    }
}
