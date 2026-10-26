#include "hardware/dht11-sensor.h"

#include <unistd.h>

#include <cstdint>
#include <iostream>

#include <pigpio.h>

Dht11Sensor::Dht11Sensor(int gpio_pin)
    : gpio_(gpio_pin), data_{}, initialized_(false), monitoring_(false) {}

Dht11Sensor::~Dht11Sensor() {
  StopMonitoring();
  Join();
  Shutdown();
}

bool Dht11Sensor::Initialize() {
  if (initialized_) {
    return true;
  }

  if (gpioInitialise() < 0) {
    std::cerr << "pigpio initialization failed\n";
    return false;
  }

  initialized_ = true;
  return true;
}

void Dht11Sensor::Shutdown() {
  if (initialized_) {
    gpioTerminate();
    initialized_ = false;
  }
}

void Dht11Sensor::StartMonitoring(int interval_seconds) {
  if (monitoring_.load()) {
    return;
  }

  if (!Initialize()) {
    return;
  }

  monitoring_.store(true);
  dht11_thread_ = std::thread([this, interval_seconds]() {
    this->Monitor(interval_seconds);
  });
}

void Dht11Sensor::StopMonitoring() { monitoring_.store(false); }

void Dht11Sensor::Join() {
  if (dht11_thread_.joinable()) {
    dht11_thread_.join();
  }
}

void Dht11Sensor::Monitor(int interval_seconds) {
  while (monitoring_.load()) {
    float temperature = 0.0f;
    float humidity = 0.0f;

    if (Read(temperature, humidity)) {
      std::cout << "Temperature: " << temperature << " C | Humidity: "
                << humidity << " %\n";
    } else {
      std::cout << "Failed to read DHT11\n";
    }

    sleep(interval_seconds);
  }
}

bool Dht11Sensor::Read(float& temperature, float& humidity) {
  for (int i = 0; i < 5; ++i) {
    data_[i] = 0;
  }

  // Send start signal.
  gpioSetMode(gpio_, PI_OUTPUT);
  gpioWrite(gpio_, 0);
  gpioDelay(18000);  // 18 ms

  gpioWrite(gpio_, 1);
  gpioDelay(40);

  gpioSetMode(gpio_, PI_INPUT);

  // Wait for DHT response.
  uint32_t timeout = 10000;

  while (gpioRead(gpio_) == 1) {
    if (--timeout == 0) {
      return false;
    }
  }

  timeout = 10000;
  while (gpioRead(gpio_) == 0) {
    if (--timeout == 0) {
      return false;
    }
  }

  timeout = 10000;
  while (gpioRead(gpio_) == 1) {
    if (--timeout == 0) {
      return false;
    }
  }

  // Read 40 bits.
  for (int i = 0; i < 40; ++i) {
    // Wait for LOW.
    timeout = 10000;
    while (gpioRead(gpio_) == 0) {
      if (--timeout == 0) {
        return false;
      }
    }

    const uint32_t start_tick = gpioTick();

    // Wait for HIGH end.
    timeout = 10000;
    while (gpioRead(gpio_) == 1) {
      if (--timeout == 0) {
        return false;
      }
    }

    const uint32_t duration = gpioTick() - start_tick;

    data_[i / 8] <<= 1;

    // >50us means bit 1.
    if (duration > 50) {
      data_[i / 8] |= 1;
    }
  }

  // Verify checksum.
  const uint8_t checksum =
      (data_[0] + data_[1] + data_[2] + data_[3]) & 0xFF;

  if (checksum != data_[4]) {
    return false;
  }

  humidity = data_[0];
  temperature = data_[2];

  return true;
}
