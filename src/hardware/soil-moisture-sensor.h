#ifndef HARDWARE_SOIL_MOISTURE_SENSOR_H_
#define HARDWARE_SOIL_MOISTURE_SENSOR_H_

#include <atomic>
#include <string>
#include <thread>

#include "communication/serial-port.h"

/// @brief Soil moisture sensor read through an ATtiny over UART.
///
/// The ATtiny samples the analog probe and reports the raw 10-bit ADC value as
/// a line, e.g. "soil:512\n" (a bare number such as "512\n" is also accepted).
/// This class requests a sample, parses the reply, converts the raw value to a
/// moisture percentage using the calibration points, and keeps the latest
/// reading available.
class SoilMoistureSensor {
 public:
  SoilMoistureSensor(SerialPort* serial_port, int raw_dry, int raw_wet);
  ~SoilMoistureSensor();

  SoilMoistureSensor(const SoilMoistureSensor&) = delete;
  SoilMoistureSensor& operator=(const SoilMoistureSensor&) = delete;

  void StartMonitoring(int interval_seconds = 5);
  void StopMonitoring();
  void Join();
  void Monitor(int interval_seconds = 5);

  /// @brief Feeds a chunk of UART data from the ATtiny. Partial lines are
  /// buffered until a newline is received.
  void ParseData(const std::string& data);

  /// @brief Returns the latest moisture reading as a percentage (0-100).
  bool Read(float& moisture_percent);

  /// @brief Returns the latest raw ADC value reported by the ATtiny.
  bool ReadRaw(int& raw_value);

  /// @brief Converts a raw ADC value to a moisture percentage (0-100).
  static float RawToPercent(int raw, int raw_dry, int raw_wet);

 private:
  bool ParseLine(const std::string& line);

  SerialPort* serial_port_;
  int raw_dry_;
  int raw_wet_;

  std::atomic<int> last_raw_;
  std::atomic<bool> has_reading_;
  std::atomic<bool> monitoring_;
  std::thread thread_;

  std::string partial_line_;
};

#endif  // HARDWARE_SOIL_MOISTURE_SENSOR_H_
