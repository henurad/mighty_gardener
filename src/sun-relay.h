#ifndef SUN_RELAY_H_
#define SUN_RELAY_H_

#include <atomic>
#include <thread>

#include "relay.h"
#include "serial-port.h"

class SunRelay {
public:
    SunRelay(SerialPort& serial_port,
             Relay& sun_light_relay,
             Relay& heater_relay,
             Relay& watering_relay);
    ~SunRelay();

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

#endif  // SUN_RELAY_H_
