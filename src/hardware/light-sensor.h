#ifndef HARDWARE_LIGHT_SENSOR_H_
#define HARDWARE_LIGHT_SENSOR_H_

#include <atomic>
#include <string>
#include <thread>

#include "communication/serial-port.h"

/// @brief Light sensor (LDR / photoresistor) read through an ATtiny over UART.
///
/// An LDR is a light-dependent variable resistor. The ATtiny samples the
/// divider voltage across it and reports the raw 10-bit ADC value as a line,
/// e.g. "light:700\n" (a bare number such as "700\n" is also accepted).
/// This class requests a sample, parses the reply, converts the raw value to a
/// light percentage using the calibration points, and keeps the latest reading
/// available.
class LightSensor {
 public:
  LightSensor(SerialPort* serial_port, int raw_dark, int raw_bright);
  ~LightSensor();

  LightSensor(const LightSensor&) = delete;
  LightSensor& operator=(const LightSensor&) = delete;

  void StartMonitoring(int interval_seconds = 5);
  void StopMonitoring();
  void Join();
  void Monitor(int interval_seconds = 5);

  /// @brief Feeds a chunk of UART data from the ATtiny. Partial lines are
  /// buffered until a newline is received.
  void ParseData(const std::string& data);

  /// @brief Returns the latest light reading as a percentage (0-100).
  bool Read(float& light_percent);

  /// @brief Returns the latest raw ADC value reported by the ATtiny.
  bool ReadRaw(int& raw_value);

  /// @brief Converts a raw ADC value to a light percentage (0-100).
  static float RawToPercent(int raw, int raw_dark, int raw_bright);

 private:
  bool ParseLine(const std::string& line);

  SerialPort* serial_port_;
  int raw_dark_;
  int raw_bright_;

  std::atomic<int> last_raw_;
  std::atomic<bool> has_reading_;
  std::atomic<bool> monitoring_;
  std::thread thread_;

  std::string partial_line_;
};

#endif  // HARDWARE_LIGHT_SENSOR_H_
