#include "hardware/am2302-sensor.h"

#include <unistd.h>

#include <cstdint>
#include <iostream>

#include <pigpio.h>

Am2302Sensor::Am2302Sensor(int gpio_pin)
    : gpio_(gpio_pin), data_{}, initialized_(false), monitoring_(false) {}

Am2302Sensor::~Am2302Sensor() {
  StopMonitoring();
  Join();
  Shutdown();
}

bool Am2302Sensor::Initialize() {
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

void Am2302Sensor::Shutdown() {
  if (initialized_) {
    gpioTerminate();
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
  for (int i = 0; i < 5; ++i) {
    data_[i] = 0;
  }

  // Send start signal.
  gpioSetMode(gpio_, PI_OUTPUT);
  gpioWrite(gpio_, 0);
  gpioDelay(1000);  // 1 ms

  gpioWrite(gpio_, 1);
  gpioDelay(40);

  gpioSetMode(gpio_, PI_INPUT);

  // Wait for AM2302 response.
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

  // AM2302 encodes humidity and temperature as signed 16-bit values scaled
  // by 10 (e.g. 235 = 23.5%). The temperature MSB is the sign bit.
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
