# Todo List
- [ ] Valve class
- [ ] Timed relay

# Raspberry Pi setup

The application can be built for both the original Raspberry Pi Zero (ARMv6)
and the Raspberry Pi Zero 2 W. Use the 32-bit Raspberry Pi OS image on both
boards so the same ARMv6-compatible userspace works across them.

Install the build tools and the GPIO libraries on the Pi:

```sh
sudo apt update
sudo apt install build-essential cmake libgpiod-dev libpigpio-dev
```

Build with the hardware backends required. This makes CMake stop with an error
if a GPIO library is missing instead of producing an application with disabled
hardware:

```sh
cmake -S . -B build -DMIGHTY_GARDENER_REQUIRE_HARDWARE=ON \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build -j2
```

The relay pins use BCM numbering through libgpiod, and the DHT11 uses pigpio.
Configure the serial console off and the UART on with `raspi-config` before
using `/dev/serial0` for the GSM module. Run the application with the required
permissions to access GPIO and the serial device.

For development and unit tests on non-Pi machines, leave
`MIGHTY_GARDENER_REQUIRE_HARDWARE` off; GPIO-dependent runtime operations are
not available in that mode.

# Features
- [ ] Automatic Watering
- [ ] Temperature control
- [ ] Humidity Control
- [ ] Tashkhise Afat
- [ ] Air conditioning
- [ ] Security and Life saving
- [ ] Fire Sensor and Alarm
- [ ] Monitoring (opencv UI)
- [ ] Mahpash
- [ ] Handler Bot
- [ ] Water Container control

## Commanding systems 
- [ ] Baleh Bot
- [ ] Telegram Bot
- [ ] Sms
- [ ] SSH 
