#ifndef HARDWARE_DHT11_SENSOR_H_
#define HARDWARE_DHT11_SENSOR_H_

#include <atomic>
#include <cstdint>
#include <thread>

class Dht11Sensor {
 public:
  explicit Dht11Sensor(int gpio_pin);
  ~Dht11Sensor();

  Dht11Sensor(const Dht11Sensor&) = delete;
  Dht11Sensor& operator=(const Dht11Sensor&) = delete;

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
  std::thread dht11_thread_;
};

#endif  // HARDWARE_DHT11_SENSOR_H_
