#ifndef COMMUNICATION_SMS_MANAGER_H_
#define COMMUNICATION_SMS_MANAGER_H_

#include <string>

#include "communication/serial-port.h"
#include "communication/sms-utils.h"

class SmsManager {
 public:
  SmsManager();
  ~SmsManager();

  static std::string ParseGsmTime(const std::string& gsm_time);
  static bool HasInternet();
  static void SetSystemTime(const std::string& datetime);
  static bool WaitForNetworkRegistration(SmsModule& sms_module,
                                         int timeout_seconds = 60);

  void Setup(SerialPort* serial_port);
  void SetAction(SmsModule::ActionCallback action);
  void ParseData(const std::string& data);
  void RunStartupSync();
  void SendReply(const std::string& text, const std::string& phone_number);
  void Call(const std::string& phone_number);
  void ResetGsm();
  std::string GetTimeFromGsm();

 private:
  SmsModule sms_module_;
};

#endif  // COMMUNICATION_SMS_MANAGER_H_
