#ifndef CONTROL_RELAY_SCHEDULER_H_
#define CONTROL_RELAY_SCHEDULER_H_

#include <atomic>
#include <thread>

#include "communication/serial-port.h"
#include "hardware/relay.h"

class RelayScheduler {
 public:
  RelayScheduler(SerialPort& serial_port, Relay& sun_light_relay,
                 Relay& heater_relay, Relay& watering_relay);
  ~RelayScheduler();

  RelayScheduler(const RelayScheduler&) = delete;
  RelayScheduler& operator=(const RelayScheduler&) = delete;

  void Start();
  void Join();

 private:
  void Run();

  SerialPort& serial_port_;
  Relay& sun_light_relay_;
  Relay& heater_relay_;
  Relay& watering_relay_;
  std::atomic<bool> running_{false};
  std::thread worker_;
};

#endif  // CONTROL_RELAY_SCHEDULER_H_
