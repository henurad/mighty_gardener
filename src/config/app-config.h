#pragma once

#include <string>

struct AppConfig {
    std::string serial_port = "/dev/serial0";
    int baud_rate = 9600;
    int dht_gpio = 4;
    int sun_light_relay_gpio = 19;
    int heater_relay_gpio = 26;
    int watering_relay_gpio = 6;
};

std::string FindAppConfigPath(const std::string& override_path = "");
AppConfig LoadAppConfig(const std::string& config_path = "");
