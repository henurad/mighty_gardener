#ifndef COMMUNICATION_SMS_UTILS_H_
#define COMMUNICATION_SMS_UTILS_H_

#include <string>

#include "communication/serial-port.h"

class SmsModule {
 public:
  using ActionCallback = void (*)(const std::string& phone_number,
                                  const std::string& msg);

  bool ValidatePhoneNumber(const char* number);

  void Setup(SerialPort* serial_port);
  void SetAction(ActionCallback action);
  void ParseData(const std::string& data);
  void ResetGsm();
  void Reply(const std::string& text, const std::string& phone_number);
  void Call(const std::string& phone_number);
  std::string GetTimeFromGsm();
  bool IsNetworkRegistered();
  void CheckSignalStrength();
  void QueryNetworkRegistration();

 private:
  static constexpr int kPhoneNumberSize = 13;
  static constexpr int kTotalPhoneNumbers = 1;

  void ClearSmsState();
  void HandleRing();
  void HandleSmsNotification(const std::string& notification);
  void HandleNetworkRegistration(const std::string& notification);
  bool ParseSmsHeaderAndBody(const std::string& header,
                             const std::string& body);
  void HandleSmsPayload(const std::string& header, const std::string& body);

  SerialPort* serial_port_ = nullptr;
  ActionCallback do_action_ = nullptr;

  std::string sender_number_;
  std::string msg_;
  bool pending_sms_header_ = false;
  std::string pending_sms_header_line_;
  std::string partial_line_;

  bool waiting_for_time_ = false;
  std::string gsm_time_;
  bool network_registered_ = false;

  char phone_numbers_[kTotalPhoneNumbers][kPhoneNumberSize + 1] = {
      "+989027732097"};
};

#endif  // COMMUNICATION_SMS_UTILS_H_
