#ifndef HARDWARE_YF_S201_H_
#define HARDWARE_YF_S201_H_

#include <atomic>
#include <cstdint>
#include <thread>

/// @todo test this module
/// @brief YF-S201 flow sensor
class YfS201 {
 public:
  explicit YfS201(int gpio_pin);
  ~YfS201();

  YfS201(const YfS201&) = delete;
  YfS201& operator=(const YfS201&) = delete;

  bool Initialize();
  void Shutdown();
  void StartMonitoring(int interval_seconds = 5);
  void StopMonitoring();
  void Join();
  void Monitor(int interval_seconds = 5);
  bool Read(float& flow_rate_lpm);

 private:
  // The YF-S201 emits ~450 pulses per litre (7.5 Hz per L/min).
  static constexpr float kPulsesPerLiter = 450.0f;
  static constexpr uint32_t kSampleWindowUs = 1000000;

  int gpio_;
  bool initialized_;
  std::atomic<bool> monitoring_;
  std::thread yf_s201_thread_;
};

#endif  // HARDWARE_YF_S201_H_
