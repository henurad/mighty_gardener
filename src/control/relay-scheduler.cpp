#include "control/relay-scheduler.h"

#include <chrono>
#include <ctime>

RelayScheduler::RelayScheduler(SerialPort& serial_port, Relay& sun_light_relay,
                               Relay& heater_relay, Relay& watering_relay)
    : serial_port_(serial_port),
      sun_light_relay_(sun_light_relay),
      heater_relay_(heater_relay),
      watering_relay_(watering_relay) {}

RelayScheduler::~RelayScheduler() {
  running_.store(false);
  Join();
}

void RelayScheduler::Start() {
  if (running_.load()) {
    return;
  }

  running_.store(true);
  worker_ = std::thread(&RelayScheduler::Run, this);
}

void RelayScheduler::Join() {
  if (worker_.joinable()) {
    worker_.join();
  }
}

void RelayScheduler::Run() {
  while (running_.load() && serial_port_.IsOpen()) {
    const std::time_t now = std::time(nullptr);
    std::tm local{};
    localtime_r(&now, &local);
    const int hour = local.tm_hour;
    const int minute = local.tm_min;

    if (hour >= 18 && hour < 24) {
      sun_light_relay_.TurnOn();
    } else {
      sun_light_relay_.TurnOff();
    }

    if (hour >= 8 && hour <= 20) {
      heater_relay_.TurnOff();
    } else {
      heater_relay_.TurnOn();
    }

    if (hour == 12 && minute < 2) {
      watering_relay_.TurnOn();
    } else {
      watering_relay_.TurnOff();
    }

    std::this_thread::sleep_for(std::chrono::seconds(10));
  }
}
