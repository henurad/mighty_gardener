#ifndef CONFIG_APP_CONFIG_H_
#define CONFIG_APP_CONFIG_H_

#include <string>

struct AppConfig {
  std::string serial_port = "/dev/serial0";
  int baud_rate = 9600;
  int dht_gpio = 4;
  std::string soil_moisture_serial_port = "";
  int soil_moisture_baud_rate = 9600;
  int soil_moisture_raw_dry = 1023;
  int soil_moisture_raw_wet = 0;
  std::string light_serial_port = "";
  int light_baud_rate = 9600;
  int light_raw_dark = 0;
  int light_raw_bright = 1023;
  int sun_light_relay_gpio = 19;
  int heater_relay_gpio = 26;
  int watering_relay_gpio = 6;
};

std::string FindAppConfigPath(const std::string& override_path = "");
AppConfig LoadAppConfig(const std::string& config_path = "");

#endif  // CONFIG_APP_CONFIG_H_
