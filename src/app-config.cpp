#include "app-config.h"

#include "mini-yaml.h"

#include <fstream>
#include <map>
#include <string>
#include <vector>

namespace {

std::vector<std::string> DefaultConfigCandidates() {
    return {
        "AppConfig.yaml",
        "./AppConfig.yaml",
        "../AppConfig.yaml",
        "../../AppConfig.yaml",
        "../src/AppConfig.yaml",
        "/home/amin/Projects/mighty_gardener/AppConfig.yaml"
    };
}

std::string ReadStringValue(const std::map<std::string, std::string>& values,
                            const std::string& key,
                            const std::string& fallback) {
    const auto it = values.find(mini_yaml::NormalizeKey(key));
    if (it == values.end() || it->second.empty()) {
        return fallback;
    }
    return it->second;
}

int ReadIntValue(const std::map<std::string, std::string>& values,
                 const std::string& key,
                 int fallback) {
    const auto it = values.find(mini_yaml::NormalizeKey(key));
    if (it == values.end() || it->second.empty()) {
        return fallback;
    }

    try {
        return std::stoi(it->second);
    } catch (const std::exception&) {
        return fallback;
    }
}

}  // namespace

std::string FindAppConfigPath(const std::string& override_path) {
    if (!override_path.empty()) {
        std::ifstream file(override_path);
        if (file.is_open()) {
            return override_path;
        }
    }

    for (const std::string& candidate : DefaultConfigCandidates()) {
        std::ifstream file(candidate);
        if (file.is_open()) {
            return candidate;
        }
    }

    return "";
}

AppConfig LoadAppConfig(const std::string& config_path) {
    AppConfig config;

    const std::string resolved_path = config_path.empty() ? FindAppConfigPath() : config_path;
    if (resolved_path.empty()) {
        return config;
    }

    const auto values = mini_yaml::LoadFile(resolved_path);
    config.serial_port = ReadStringValue(values, "SERIAL_PORT", config.serial_port);
    config.baud_rate = ReadIntValue(values, "BAUD_RATE", config.baud_rate);
    config.dht_gpio = ReadIntValue(values, "DHT_GPIO", config.dht_gpio);
    config.sun_light_relay_gpio = ReadIntValue(values, "SUN_LIGHT_RELAY_GPIO", config.sun_light_relay_gpio);
    config.heater_relay_gpio = ReadIntValue(values, "HEATER_RELAY_GPIO", config.heater_relay_gpio);
    config.watering_relay_gpio = ReadIntValue(values, "WATERING_RELAY_GPIO", config.watering_relay_gpio);

    return config;
}
