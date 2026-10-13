#include "hardware/light-sensor.h"

#include <unistd.h>

#include <exception>
#include <iostream>
#include <string>

namespace {

std::string Trim(const std::string& str) {
  const std::size_t begin = str.find_first_not_of(" \t\r\n");
  if (begin == std::string::npos) {
    return "";
  }
  const std::size_t end = str.find_last_not_of(" \t\r\n");
  return str.substr(begin, end - begin + 1);
}

}  // namespace

LightSensor::LightSensor(SerialPort* serial_port, int raw_dark, int raw_bright)
    : serial_port_(serial_port),
      raw_dark_(raw_dark),
      raw_bright_(raw_bright),
      last_raw_(0),
      has_reading_(false),
      monitoring_(false) {}

LightSensor::~LightSensor() {
  StopMonitoring();
  Join();
}

float LightSensor::RawToPercent(int raw, int raw_dark, int raw_bright) {
  if (raw_dark == raw_bright) {
    return 0.0f;
  }

  const float percent = 100.0f * static_cast<float>(raw - raw_dark) /
                        static_cast<float>(raw_bright - raw_dark);

  if (percent < 0.0f) {
    return 0.0f;
  }
  if (percent > 100.0f) {
    return 100.0f;
  }
  return percent;
}

void LightSensor::StartMonitoring(int interval_seconds) {
  if (monitoring_.load()) {
    return;
  }

  monitoring_.store(true);
  thread_ = std::thread([this, interval_seconds]() {
    this->Monitor(interval_seconds);
  });
}

void LightSensor::StopMonitoring() { monitoring_.store(false); }

void LightSensor::Join() {
  if (thread_.joinable()) {
    thread_.join();
  }
}

void LightSensor::Monitor(int interval_seconds) {
  while (monitoring_.load()) {
    if (serial_port_ != nullptr) {
      serial_port_->Println("read");
    }

    float light_percent = 0.0f;
    int raw_value = 0;

    if (Read(light_percent) && ReadRaw(raw_value)) {
      std::cout << "Light: " << light_percent << " % (raw " << raw_value
                << ")\n";
    } else {
      std::cout << "No light reading yet\n";
    }

    sleep(interval_seconds);
  }
}

void LightSensor::ParseData(const std::string& data) {
  partial_line_ += data;

  std::size_t newline = partial_line_.find('\n');
  while (newline != std::string::npos) {
    const std::string line = partial_line_.substr(0, newline);
    ParseLine(line);
    partial_line_.erase(0, newline + 1);
    newline = partial_line_.find('\n');
  }
}

bool LightSensor::ParseLine(const std::string& line) {
  const std::string trimmed = Trim(line);
  if (trimmed.empty()) {
    return false;
  }

  // Accept both "light:<raw>" and a bare "<raw>".
  std::string value = trimmed;
  const std::size_t colon = trimmed.find(':');
  if (colon != std::string::npos) {
    value = Trim(trimmed.substr(colon + 1));
  }

  int raw = 0;
  try {
    raw = std::stoi(value);
  } catch (const std::exception&) {
    return false;
  }

  last_raw_.store(raw);
  has_reading_.store(true);
  return true;
}

bool LightSensor::Read(float& light_percent) {
  if (!has_reading_.load()) {
    return false;
  }

  light_percent = RawToPercent(last_raw_.load(), raw_dark_, raw_bright_);
  return true;
}

bool LightSensor::ReadRaw(int& raw_value) {
  if (!has_reading_.load()) {
    return false;
  }

  raw_value = last_raw_.load();
  return true;
}
