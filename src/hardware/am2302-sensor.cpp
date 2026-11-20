#include "hardware/am2302-sensor.h"

#include <unistd.h>

#include <cstdint>
#include <iostream>

#include <chrono>
#include <thread>

Am2302Sensor::Am2302Sensor(int gpio_pin)
    : line_(gpio_pin, GpioLine::Direction::kInput),
      data_{}, initialized_(false), monitoring_(false) {}

Am2302Sensor::~Am2302Sensor() {
  StopMonitoring();
  Join();
  Shutdown();
}

bool Am2302Sensor::Initialize() {
  if (initialized_) {
    return true;
  }

  if (!line_.Open()) {
    return false;
  }

  initialized_ = true;
  return true;
}

void Am2302Sensor::Shutdown() {
  if (initialized_) {
    line_.Close();
    initialized_ = false;
  }
}

void Am2302Sensor::StartMonitoring(int interval_seconds) {
  if (monitoring_.load()) {
    return;
  }

  if (!Initialize()) {
    return;
  }

  monitoring_.store(true);
  am2302_thread_ = std::thread([this, interval_seconds]() {
    this->Monitor(interval_seconds);
  });
}

void Am2302Sensor::StopMonitoring() { monitoring_.store(false); }

void Am2302Sensor::Join() {
  if (am2302_thread_.joinable()) {
    am2302_thread_.join();
  }
}

void Am2302Sensor::Monitor(int interval_seconds) {
  while (monitoring_.load()) {
    float temperature = 0.0f;
    float humidity = 0.0f;

    if (Read(temperature, humidity)) {
      std::cout << "Temperature: " << temperature << " C | Humidity: "
                << humidity << " %\n";
    } else {
      std::cout << "Failed to read AM2302\n";
    }

    sleep(interval_seconds);
  }
}

bool Am2302Sensor::Read(float& temperature, float& humidity) {
  for (int i = 0; i < 5; ++i) data_[i] = 0;

  if (!line_.SetDirection(GpioLine::Direction::kOutput, false)) return false;
  usleep(1000);
  if (!line_.SetValue(true)) return false;
  usleep(40);
  if (!line_.SetDirection(GpioLine::Direction::kInput)) return false;

  auto wait_level = [this](bool expected) {
    const auto deadline = std::chrono::steady_clock::now() +
                          std::chrono::milliseconds(10);
    bool level = false;
    do {
      if (!line_.Read(level)) return false;
      if (level == expected) return true;
    } while (std::chrono::steady_clock::now() < deadline);
    return false;
  };

  if (!wait_level(false) || !wait_level(true) || !wait_level(false)) {
    return false;
  }

  for (int i = 0; i < 40; ++i) {
    if (!wait_level(true)) return false;

    const auto start = std::chrono::steady_clock::now();
    if (!wait_level(false)) return false;

    const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                              std::chrono::steady_clock::now() - start)
                              .count();
    data_[i / 8] <<= 1;
    if (duration > 50) data_[i / 8] |= 1;
  }

  const uint8_t checksum =
      (data_[0] + data_[1] + data_[2] + data_[3]) & 0xFF;
  if (checksum != data_[4]) return false;

  const int16_t raw_humidity =
      static_cast<int16_t>((data_[0] << 8) | data_[1]);
  int16_t raw_temperature = static_cast<int16_t>((data_[2] << 8) | data_[3]);
  if (raw_temperature & 0x8000) {
    raw_temperature = -(raw_temperature & 0x7FFF);
  }

  humidity = raw_humidity / 10.0f;
  temperature = raw_temperature / 10.0f;
  return true;
}
