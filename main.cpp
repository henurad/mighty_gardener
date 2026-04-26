#include "serial_port.h"
#include <cstring>
#include <iostream>
#include <cstdio>
#include <thread>
#include <chrono>
#include <ctime>
#include "sms_utils.h"
#include "Relay.h"

SmsModule sms_module;

class MySerial : public SerialPort {
    void on_received(const char* data, size_t size) override {
        std::string str(data, size);
        std::printf("%s", str.c_str());
        sms_module.ParseData(str);
    }
};

void DoAction(std::string phoneNumber, std::string msg) {
    if (msg == "callme") {
        sms_module.Call(phoneNumber);
    } else if (msg == "ping") {
        sms_module.Reply("pong", phoneNumber);
    } else if (msg == "rstgsm") {
        sms_module.ResetGsm();
    } else if (msg == "v1 on") {
        //watering_module.OpenValve1();
        //valve1_timer.Start();
        sms_module.Reply("valve1 opened", phoneNumber);
    } else if (msg == "v1 off") {
        //watering_module.CloseValve1();
        //valve1_timer.Stop();
        sms_module.Reply("valve1 closed", phoneNumber);
    } else if (msg == "v2 on") {
        //watering_module.OpenValve2();
        //valve2_timer.Start();
        sms_module.Reply("valve2 opened", phoneNumber);
    } else if (msg == "v2 off") {
        //watering_module.CloseValve2();
        //valve2_timer.Stop();
        sms_module.Reply("valve2 closed", phoneNumber);
    } else if (msg == "status") {
        /// @todo
    } else if (msg == "reboot") {
        sms_module.Reply("Rebooting system...", phoneNumber);
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
        sms_module.Reply(helpMsg, phoneNumber);
    } else {
        sms_module.Reply("Unknown command: " + msg, phoneNumber);
    }
}

int main() {
    MySerial sp;
    if (!sp.open((char*)"/dev/serial0", 9600)) {
        return 1;
    }

    sms_module.Setup(&sp);
    sms_module.SetAction(DoAction);

    // Sun light relay: GPIO 19, active-low. Turns ON nightly from 20:00 to 24:00 (midnight).
    Relay sun_light_relay(19, true);
    Relay heater_relay(26, true);
    std::thread sunRelayThread([&sp, &sun_light_relay, &heater_relay]() {
        while (sp.isOpen()) {
            std::time_t t = std::time(nullptr);
            std::tm local = *std::localtime(&t);
            int hour = local.tm_hour;
            if (hour >= 18 && hour < 24) {
                sun_light_relay.turnOn();
            } else {
                sun_light_relay.turnOff();
            }

            if (hour >= 8 && hour <= 20) {
		    heater_relay.turnOff();
	    } else {
		    heater_relay.turnOn();
	    }


            std::this_thread::sleep_for(std::chrono::seconds(30));
        }
    });
    sunRelayThread.detach();

    while (sp.isOpen()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}

