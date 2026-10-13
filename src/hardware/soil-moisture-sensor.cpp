#include "hardware/soil-moisture-sensor.h"

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

SoilMoistureSensor::SoilMoistureSensor(SerialPort* serial_port, int raw_dry,
                                       int raw_wet)
    : serial_port_(serial_port),
      raw_dry_(raw_dry),
      raw_wet_(raw_wet),
      last_raw_(0),
      has_reading_(false),
      monitoring_(false) {}

SoilMoistureSensor::~SoilMoistureSensor() {
  StopMonitoring();
  Join();
}

float SoilMoistureSensor::RawToPercent(int raw, int raw_dry, int raw_wet) {
  if (raw_dry == raw_wet) {
    return 0.0f;
  }

  const float percent = 100.0f * static_cast<float>(raw_dry - raw) /
                        static_cast<float>(raw_dry - raw_wet);

  if (percent < 0.0f) {
    return 0.0f;
  }
  if (percent > 100.0f) {
    return 100.0f;
  }
  return percent;
}

void SoilMoistureSensor::StartMonitoring(int interval_seconds) {
  if (monitoring_.load()) {
    return;
  }

  monitoring_.store(true);
  thread_ = std::thread([this, interval_seconds]() {
    this->Monitor(interval_seconds);
  });
}

void SoilMoistureSensor::StopMonitoring() { monitoring_.store(false); }

void SoilMoistureSensor::Join() {
  if (thread_.joinable()) {
    thread_.join();
  }
}

void SoilMoistureSensor::Monitor(int interval_seconds) {
  while (monitoring_.load()) {
    if (serial_port_ != nullptr) {
      serial_port_->Println("read");
    }

    float moisture_percent = 0.0f;
    int raw_value = 0;

    if (Read(moisture_percent) && ReadRaw(raw_value)) {
      std::cout << "Soil moisture: " << moisture_percent << " % (raw "
                << raw_value << ")\n";
    } else {
      std::cout << "No soil moisture reading yet\n";
    }

    sleep(interval_seconds);
  }
}

void SoilMoistureSensor::ParseData(const std::string& data) {
  partial_line_ += data;

  std::size_t newline = partial_line_.find('\n');
  while (newline != std::string::npos) {
    const std::string line = partial_line_.substr(0, newline);
    ParseLine(line);
    partial_line_.erase(0, newline + 1);
    newline = partial_line_.find('\n');
  }
}

bool SoilMoistureSensor::ParseLine(const std::string& line) {
  const std::string trimmed = Trim(line);
  if (trimmed.empty()) {
    return false;
  }

  // Accept both "soil:<raw>" and a bare "<raw>".
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

bool SoilMoistureSensor::Read(float& moisture_percent) {
  if (!has_reading_.load()) {
    return false;
  }

  moisture_percent = RawToPercent(last_raw_.load(), raw_dry_, raw_wet_);
  return true;
}

bool SoilMoistureSensor::ReadRaw(int& raw_value) {
  if (!has_reading_.load()) {
    return false;
  }

  raw_value = last_raw_.load();
  return true;
}
