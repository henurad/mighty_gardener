#include "serial_port.h"
#include <cstring>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <thread>
#include <chrono>
#include <ctime>
#include "sms_utils.h"
#include "Relay.h"

SmsModule sms_module;

bool hasInternet() {
    return system("ping -c 1 -W 1 8.8.8.8 > /dev/null 2>&1") == 0;
}

std::string parseGsmTime(const std::string& gsm_time) {
    // +CCLK: "24/05/01,12:34:56+08"
    // Convert to Tehran time (UTC+3:30)
    size_t start = gsm_time.find('"');
    if (start == std::string::npos) return "";
    start++;
    size_t end = gsm_time.find('"', start);
    if (end == std::string::npos) return "";
    std::string time_str = gsm_time.substr(start, end - start);
    // 24/05/01,12:34:56+08
    size_t comma = time_str.find(',');
    if (comma == std::string::npos) return "";
    std::string date = time_str.substr(0, comma);
    std::string time_part = time_str.substr(comma + 1);
    
    // date 24/05/01 -> 2024-05-01
    std::string year = "20" + date.substr(0, 2);
    std::string month = date.substr(3, 2);
    std::string day = date.substr(6, 2);
    
    // Parse time components
    size_t colon1 = time_part.find(':');
    size_t colon2 = time_part.find(':', colon1 + 1);
    if (colon1 == std::string::npos || colon2 == std::string::npos) return "";
    
    int hour = 0, minute = 0, second = 0;
    try {
        hour = std::stoi(time_part.substr(0, colon1));
        minute = std::stoi(time_part.substr(colon1 + 1, colon2 - colon1 - 1));
        second = std::stoi(time_part.substr(colon2 + 1));
    } catch (...) {
        return ""; // Invalid time format
    }
    
    // Validate ranges
    if (hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 59) {
        return ""; // Invalid time values
    }
    
    // Format final time string safely
    char time_buf[9];
    snprintf(time_buf, sizeof(time_buf), "%02d:%02d:%02d", hour, minute, second);
    
    return year + "-" + month + "-" + day + " " + time_buf;
}

void setSystemTime(const std::string& datetime) {
    std::string cmd = "date -s '" + datetime + "'";
    std::cout << "cmd is: " << cmd << std::endl << std::flush;
    system(cmd.c_str());
}

bool waitForNetworkRegistration(SmsModule& sms_module, int timeout_seconds = 60) {
    printf("Waiting for GSM network registration (timeout: %d seconds)...\n", timeout_seconds);
    
    // Check signal strength first
    sms_module.CheckSignalStrength();
    msleep(500);
    
    int query_interval = 5; // Query every 5 seconds
    int queries = 0;
    
    for (int i = 0; i < timeout_seconds * 2; ++i) {  // Check every 500ms
        if (sms_module.IsNetworkRegistered()) {
            printf("GSM network registered successfully.\n");
            return true;
        }
        
        // Query registration status every few seconds
        if (i % (query_interval * 2) == 0) {
            sms_module.QueryNetworkRegistration();
            queries++;
            printf("Querying network registration status (attempt %d)...\n", queries);
        }
        
        usleep(500000);  // 500ms
    }
    printf("GSM network registration timeout after %d seconds.\n", timeout_seconds);
    return false;
}

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

    if (!hasInternet()) {
        printf("No internet. Checking GSM network registration...\n");
        if (waitForNetworkRegistration(sms_module)) {
            std::string gsm_time_str = sms_module.GetTimeFromGsm();
            if (!gsm_time_str.empty()) {
                std::string parsed_time = parseGsmTime(gsm_time_str);
                if (!parsed_time.empty()) {
                    setSystemTime(parsed_time);
                    printf("System time updated.\n");
                } else {
                    printf("Failed to parse GSM time.\n");
                }
            } else {
                printf("GSM time is empty - no response received.\n");
            }
        } else {
            printf("Cannot get time - GSM not registered on network.\n");
        }
    } else {
        printf("Internet is available, skipping GSM time sync.\n");
    }

    // Sun light relay: GPIO 19, active-low. Turns ON nightly from 20:00 to 24:00 (midnight).
    Relay sun_light_relay(19, true);
    Relay heater_relay(26, true);
    Relay watering_relay(6, true);
    std::thread sunRelayThread([&sp, &sun_light_relay, &heater_relay, &watering_relay]() {
        while (sp.isOpen()) {
            std::time_t t = std::time(nullptr);
            std::tm local = *std::localtime(&t);
            int hour = local.tm_hour;
            int minute = local.tm_min;
            if (hour >= 18 && hour < 24) {
                sun_light_relay.turnOn();
                //printf("sun light relay turned on\n");
            } else {
                sun_light_relay.turnOff();
                //printf("sun light relay turned off\n");
            }

            if (hour >= 8 && hour <= 20) {
                heater_relay.turnOff();
                //printf("heater relay turned off\n");
            } else {
                heater_relay.turnOn();
                //printf("heater relay turned on\n");
            }

            if(hour == 12 && minute < 2) {
                watering_relay.turnOn();
                //printf("watering relay turned on\n");
            } else {
                watering_relay.turnOff();
                //printf("watering relay turned off\n");
            }
            std::this_thread::sleep_for(std::chrono::seconds(10));
        }
    });
    sunRelayThread.detach();

    while (sp.isOpen()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}

