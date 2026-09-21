#include <serial-port.h>
#include <cstring>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <ctime>

#include <dht11-sensor.h>
#include <relay.h>
#include <sms-manager.h>
#include <sun-relay.h>

#define DHT_GPIO 4

SmsManager* g_sms_manager = nullptr;

class MySerial : public SerialPort {
    void on_received(const char* data, size_t size) override {
        std::string str(data, size);
        std::printf("%s", str.c_str());
        if (g_sms_manager) {
            g_sms_manager->ParseData(str);
        }
    }
};

void DoAction(std::string phoneNumber, std::string msg) {
    if (msg == "callme") {
        g_sms_manager->Call(phoneNumber);
    } else if (msg == "ping") {
        g_sms_manager->SendReply("pong", phoneNumber);
    } else if (msg == "rstgsm") {
        g_sms_manager->ResetGsm();
    } else if (msg == "v1 on") {
        g_sms_manager->SendReply("valve1 opened", phoneNumber);
    } else if (msg == "v1 off") {
        g_sms_manager->SendReply("valve1 closed", phoneNumber);
    } else if (msg == "v2 on") {
        g_sms_manager->SendReply("valve2 opened", phoneNumber);
    } else if (msg == "v2 off") {
        g_sms_manager->SendReply("valve2 closed", phoneNumber);
    } else if (msg == "status") {
        /// @todo
    } else if (msg == "reboot") {
        g_sms_manager->SendReply("Rebooting system...", phoneNumber);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        system("sudo reboot");
    } else if (msg == "help") {
        std::string helpMsg = "Available commands:\n"
                              "callme - Call your number\n"
                              "ping - Check if system is responsive\n"
                              "rstgsm - Reset GSM module\n"
                              "v1 on/off - Open/Close valve 1\n"
                              "v2 on/off - Open/Close valve 2\n"
                              "status - Get system status\n"
                              "reboot - Reboot the system\n"
                              "help - Show this help message";
        g_sms_manager->SendReply(helpMsg, phoneNumber);
    } else {
        g_sms_manager->SendReply("Unknown command: " + msg, phoneNumber);
    }
}

int main() {
    MySerial sp;
    if (!sp.open((char*)"/dev/serial0", 9600)) {
        return 1;
    }

    SmsManager sms_manager;
    g_sms_manager = &sms_manager;
    sms_manager.Setup(&sp);
    sms_manager.SetAction(DoAction);
    sms_manager.RunStartupSync();

    Relay sun_light_relay(19, true);
    Relay heater_relay(26, true);
    Relay watering_relay(6, true);
    SunRelay sun_relay(sp, sun_light_relay, heater_relay, watering_relay);
    sun_relay.Start();

    DHT11Sensor dht(DHT_GPIO);
    if (!dht.initialize()) {
        return 1;
    }

    dht.startMonitoring(5);

    while (sp.isOpen()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    dht.stopMonitoring();
    dht.join();
    dht.shutdown();
    sun_relay.Join();

    g_sms_manager = nullptr;
    return 0;
}
