#include <gtest/gtest.h>

#include <cstdio>
#include <fstream>
#include <string>

#include "app-config.h"

TEST(AppConfigTest, LoadsValuesFromYamlFile) {
    const std::string file_path = "test-app-config.yaml";
    std::ofstream config(file_path);
    config << "serial_port: \"/dev/ttyUSB0\"\n";
    config << "baud_rate: 115200\n";
    config << "dht_gpio: 17\n";
    config << "sun_light_relay_gpio: 19\n";
    config << "heater_relay_gpio: 26\n";
    config << "watering_relay_gpio: 6\n";
    config.close();

    const AppConfig config_values = LoadAppConfig(file_path);

    EXPECT_EQ(config_values.serial_port, "/dev/ttyUSB0");
    EXPECT_EQ(config_values.baud_rate, 115200);
    EXPECT_EQ(config_values.dht_gpio, 17);
    EXPECT_EQ(config_values.sun_light_relay_gpio, 19);
    EXPECT_EQ(config_values.heater_relay_gpio, 26);
    EXPECT_EQ(config_values.watering_relay_gpio, 6);

    std::remove(file_path.c_str());
}
