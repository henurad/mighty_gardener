#ifndef SMS_MANAGER_H_
#define SMS_MANAGER_H_

#include <string>
#include <vector>

#include "serial-port.h"
#include "sms-utils.h"

class SmsManager {
public:
    static std::string ParseGsmTime(const std::string& gsm_time);
    static bool HasInternet();
    static void SetSystemTime(const std::string& datetime);
    static bool WaitForNetworkRegistration(SmsModule& sms_module, int timeout_seconds = 60);

    SmsManager();
    ~SmsManager();

    void Setup(SerialPort* serial_port);
    void SetAction(void (*action)(std::string, std::string));
    void ParseData(const std::string& data);
    void RunStartupSync();
    void SendReply(const std::string& text, const std::string& phone_number);
    void Call(const std::string& phone_number);
    void ResetGsm();
    std::string GetTimeFromGsm();

private:
    SmsModule sms_module_;
};

#endif  // SMS_MANAGER_H_
