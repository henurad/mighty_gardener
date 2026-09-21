#include "sms-manager.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <thread>
#include <unistd.h>

namespace {
std::string parseGsmTimeInternal(const std::string& gsm_time) {
    size_t start = gsm_time.find('"');
    if (start == std::string::npos) {
        return "";
    }
    ++start;
    size_t end = gsm_time.find('"', start);
    if (end == std::string::npos) {
        return "";
    }

    std::string time_str = gsm_time.substr(start, end - start);
    size_t comma = time_str.find(',');
    if (comma == std::string::npos) {
        return "";
    }

    std::string date = time_str.substr(0, comma);
    std::string time_part = time_str.substr(comma + 1);

    if (date.size() < 8) {
        return "";
    }

    std::string year = "20" + date.substr(0, 2);
    std::string month = date.substr(3, 2);
    std::string day = date.substr(6, 2);

    size_t colon1 = time_part.find(':');
    size_t colon2 = time_part.find(':', colon1 + 1);
    if (colon1 == std::string::npos || colon2 == std::string::npos) {
        return "";
    }

    int hour = 0;
    int minute = 0;
    int second = 0;

    try {
        hour = std::stoi(time_part.substr(0, colon1));
        minute = std::stoi(time_part.substr(colon1 + 1, colon2 - colon1 - 1));
        second = std::stoi(time_part.substr(colon2 + 1));
    } catch (...) {
        return "";
    }

    if (hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 59) {
        return "";
    }

    char time_buf[9];
    std::snprintf(time_buf, sizeof(time_buf), "%02d:%02d:%02d", hour, minute, second);
    return year + "-" + month + "-" + day + " " + time_buf;
}
}  // namespace

std::string SmsManager::ParseGsmTime(const std::string& gsm_time) {
    return parseGsmTimeInternal(gsm_time);
}

bool SmsManager::HasInternet() {
    return std::system("ping -c 1 -W 1 8.8.8.8 > /dev/null 2>&1") == 0;
}

void SmsManager::SetSystemTime(const std::string& datetime) {
    std::string cmd = "date -s '" + datetime + "'";
    std::cout << "cmd is: " << cmd << std::endl << std::flush;
    std::system(cmd.c_str());
}

bool SmsManager::WaitForNetworkRegistration(SmsModule& sms_module, int timeout_seconds) {
    std::printf("Waiting for GSM network registration (timeout: %d seconds)...\n", timeout_seconds);
    sms_module.CheckSignalStrength();
    msleep(500);

    int query_interval = 5;
    int queries = 0;

    for (int i = 0; i < timeout_seconds * 2; ++i) {
        if (sms_module.IsNetworkRegistered()) {
            std::printf("GSM network registered successfully.\n");
            return true;
        }

        if (i % (query_interval * 2) == 0) {
            sms_module.QueryNetworkRegistration();
            queries++;
            std::printf("Querying network registration status (attempt %d)...\n", queries);
        }

        usleep(500000);
    }

    std::printf("GSM network registration timeout after %d seconds.\n", timeout_seconds);
    return false;
}

SmsManager::SmsManager() = default;

SmsManager::~SmsManager() = default;

void SmsManager::Setup(SerialPort* serial_port) {
    sms_module_.Setup(serial_port);
}

void SmsManager::SetAction(void (*action)(std::string, std::string)) {
    sms_module_.SetAction(action);
}

void SmsManager::ParseData(const std::string& data) {
    sms_module_.ParseData(data);
}

void SmsManager::RunStartupSync() {
    if (!HasInternet()) {
        std::printf("No internet. Checking GSM network registration...\n");
        if (WaitForNetworkRegistration(sms_module_)) {
            std::string gsm_time_str = sms_module_.GetTimeFromGsm();
            if (!gsm_time_str.empty()) {
                std::string parsed_time = ParseGsmTime(gsm_time_str);
                if (!parsed_time.empty()) {
                    SetSystemTime(parsed_time);
                    std::printf("System time updated.\n");
                } else {
                    std::printf("Failed to parse GSM time.\n");
                }
            } else {
                std::printf("GSM time is empty - no response received.\n");
            }
        } else {
            std::printf("Cannot get time - GSM not registered on network.\n");
        }
    } else {
        std::printf("Internet is available, skipping GSM time sync.\n");
    }
}

void SmsManager::SendReply(const std::string& text, const std::string& phone_number) {
    sms_module_.Reply(text, phone_number);
}

void SmsManager::Call(const std::string& phone_number) {
    sms_module_.Call(phone_number);
}

void SmsManager::ResetGsm() {
    sms_module_.ResetGsm();
}

std::string SmsManager::GetTimeFromGsm() {
    return sms_module_.GetTimeFromGsm();
}
