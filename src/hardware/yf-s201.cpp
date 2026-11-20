#include "hardware/yf-s201.h"

#include <unistd.h>

#include <cstdint>
#include <iostream>

#include <chrono>
#include <thread>

YfS201::YfS201(int gpio_pin)
    : line_(gpio_pin, GpioLine::Direction::kInput),
      initialized_(false), monitoring_(false) {}

YfS201::~YfS201() {
  StopMonitoring();
  Join();
  Shutdown();
}

bool YfS201::Initialize() {
  if (initialized_) {
    return true;
  }

  if (!line_.Open()) {
    return false;
  }

  initialized_ = true;
  return true;
}

void YfS201::Shutdown() {
  if (initialized_) {
    line_.Close();
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
  const auto start = std::chrono::steady_clock::now();
  bool last_level = false;
  if (!line_.Read(last_level)) return false;

  uint32_t pulses = 0;
  while (std::chrono::duration_cast<std::chrono::microseconds>(
             std::chrono::steady_clock::now() - start)
             .count() < kSampleWindowUs) {
    bool level = false;
    if (!line_.Read(level)) return false;
    if (level && !last_level) ++pulses;
    last_level = level;
  }

  const float frequency =
      static_cast<float>(pulses) * 1000000.0f / kSampleWindowUs;
  flow_rate_lpm = frequency * 60.0f / kPulsesPerLiter;
  return true;
}
