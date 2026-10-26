#include "hardware/yf-s201.h"

#include <unistd.h>

#include <cstdint>
#include <iostream>

#include <pigpio.h>

YfS201::YfS201(int gpio_pin)
    : gpio_(gpio_pin), initialized_(false), monitoring_(false) {}

YfS201::~YfS201() {
  StopMonitoring();
  Join();
  Shutdown();
}

bool YfS201::Initialize() {
  if (initialized_) {
    return true;
  }

  if (gpioInitialise() < 0) {
    std::cerr << "pigpio initialization failed\n";
    return false;
  }

  gpioSetMode(gpio_, PI_INPUT);
  initialized_ = true;
  return true;
}

void YfS201::Shutdown() {
  if (initialized_) {
    gpioTerminate();
    initialized_ = false;
  }
}

void YfS201::StartMonitoring(int interval_seconds) {
  if (monitoring_.load()) {
    return;
  }

  if (!Initialize()) {
    return;
  }

  monitoring_.store(true);
  yf_s201_thread_ = std::thread([this, interval_seconds]() {
    this->Monitor(interval_seconds);
  });
}

void YfS201::StopMonitoring() { monitoring_.store(false); }

void YfS201::Join() {
  if (yf_s201_thread_.joinable()) {
    yf_s201_thread_.join();
  }
}

void YfS201::Monitor(int interval_seconds) {
  while (monitoring_.load()) {
    float flow_rate_lpm = 0.0f;

    if (Read(flow_rate_lpm)) {
      std::cout << "Flow rate: " << flow_rate_lpm << " L/min\n";
    } else {
      std::cout << "Failed to read YF-S201\n";
    }

    sleep(interval_seconds);
  }
}

bool YfS201::Read(float& flow_rate_lpm) {
  const uint32_t start_tick = gpioTick();
  int last_level = gpioRead(gpio_);
  uint32_t pulses = 0;

  // Count rising edges over a fixed sampling window.
  while ((gpioTick() - start_tick) < kSampleWindowUs) {
    const int level = gpioRead(gpio_);
    if (level == 1 && last_level == 0) {
      ++pulses;
    }
    last_level = level;
  }

  // Pulses per second (Hz) -> litres per minute.
  const float frequency =
      static_cast<float>(pulses) * 1000000.0f / kSampleWindowUs;
  flow_rate_lpm = frequency * 60.0f / kPulsesPerLiter;

  return true;
}
