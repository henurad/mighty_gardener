#ifndef HEADER_SMS_UTILS_H_
#define HEADER_SMS_UTILS_H_

#include "time_consts.h"
#include "serial_port.h"
#include <unistd.h>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

inline std::string trimCopy(const std::string& value) {
    auto start = value.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    auto end = value.find_last_not_of(" \t\r\n");
    return value.substr(start, end - start + 1);
}

inline void trim(std::string& value) {
    value = trimCopy(value);
}

inline void toLowerCase(std::string& value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
}

inline bool startsWith(const std::string& value, const std::string& prefix) {
    return value.size() >= prefix.size() && value.compare(0, prefix.size(), prefix) == 0;
}

inline std::vector<std::string> splitQuotedFields(const std::string& value) {
    std::vector<std::string> fields;
    std::string current;
    bool inQuotes = false;

    for (char c : value) {
        if (c == '"') {
            if (inQuotes) {
                fields.push_back(current);
                current.clear();
            }
            inQuotes = !inQuotes;
            continue;
        }

        if (!inQuotes && c == ',') {
            if (!current.empty()) {
                trim(current);
                fields.push_back(current);
                current.clear();
            }
            continue;
        }

        current.push_back(c);
    }

    if (!current.empty()) {
        trim(current);
        fields.push_back(current);
    }

    return fields;
}

inline bool fileExists(const std::string& path) {
    return access(path.c_str(), F_OK) == 0;
}

inline bool writeFile(const std::string& path, const std::string& content) {
    std::ofstream out(path);
    if (!out.is_open()) {
        return false;
    }
    out << content;
    return true;
}

inline std::string readFile(const std::string& path) {
    std::ifstream in(path);
    if (!in.is_open()) {
        return "";
    }
    std::string value;
    std::getline(in, value);
    return value;
}

inline bool ensureGpioExported(int pin) {
    std::string gpioPath = "/sys/class/gpio/gpio" + std::to_string(pin);
    if (fileExists(gpioPath)) {
        return true;
    }
    return writeFile("/sys/class/gpio/export", std::to_string(pin));
}

inline bool setGpioDirection(int pin, const std::string& direction) {
    std::string path = "/sys/class/gpio/gpio" + std::to_string(pin) + "/direction";
    return writeFile(path, direction);
}

inline bool digitalRead(int pin) {
    if (!ensureGpioExported(pin)) {
        return false;
    }
    setGpioDirection(pin, "in");
    std::string path = "/sys/class/gpio/gpio" + std::to_string(pin) + "/value";
    std::string value = readFile(path);
    return value == "1";
}

inline int analogRead(int pin) {
    (void)pin;
    /// @todo
    // Raspberry Pi does not have a native analog input.
    // Add an external ADC if you need real analog readings.
    return 512;
}

inline void msleep(int milliseconds) {
    usleep(milliseconds * 1000);
}

struct SmsModule {
private:
    SerialPort* serial_port;
    const int kTotalPhoneNo = 1;
    const int kGsmResetPin = 4;
    static const int kPhoneNumberSize = 13;
    void (*do_action)(std::string phone_number, std::string msg) = nullptr;

    std::string sms_status = "";
    std::string sender_number = "";
    std::string recieved_date = "";
    std::string msg = "";
    bool pending_sms_header = false;
    std::string pending_sms_header_line;
    std::string partial_line;

    bool waiting_for_time = false;
    std::string gsm_time = "";
    bool network_registered = false;
    bool checking_signal = false;

    bool ValidatePhoneNumber(const char* number);
    void ClearSmsState();
    void HandleRing();
    void HandleSmsNotification(const std::string& notification);
    void HandleNetworkRegistration(const std::string& notification);
    void HandleClip(const std::string& notification);
    bool ParseSmsHeaderAndBody(const std::string& header, const std::string& body);
    void HandleSmsPayload(const std::string& header, const std::string& body);

public:
    char phone_numbers[1][kPhoneNumberSize + 1] = {"+989027732097"};
    
    void Setup(SerialPort* serial_port);
    void SetAction(void (*action)(std::string, std::string));
    void ParseData(const std::string& buff);
    void ResetGsm();
    void Reply(const std::string& text, const std::string& Phone);
    void Call(const std::string& Phone);
    std::string GetTimeFromGsm();
    bool IsNetworkRegistered();
    void CheckSignalStrength();
    void QueryNetworkRegistration();
};

#endif  // HEADER_SMS_UTILS_H_
