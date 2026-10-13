#ifndef HARDWARE_AM2302_SENSOR_H_
#define HARDWARE_AM2302_SENSOR_H_

#include <atomic>
#include <cstdint>
#include <thread>

/// @todo: Test this module
class Am2302Sensor {
 public:
  explicit Am2302Sensor(int gpio_pin);
  ~Am2302Sensor();

  Am2302Sensor(const Am2302Sensor&) = delete;
  Am2302Sensor& operator=(const Am2302Sensor&) = delete;

  bool Initialize();
  void Shutdown();
  void StartMonitoring(int interval_seconds = 5);
  void StopMonitoring();
  void Join();
  void Monitor(int interval_seconds = 5);
  bool Read(float& temperature, float& humidity);

 private:
  int gpio_;
  uint8_t data_[5];
  bool initialized_;
  std::atomic<bool> monitoring_;
  std::thread am2302_thread_;
};

#endif  // HARDWARE_AM2302_SENSOR_H_
