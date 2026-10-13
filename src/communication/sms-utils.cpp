#include "communication/sms-utils.h"

#include <unistd.h>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <string>
#include <thread>
#include <vector>

namespace {

std::string TrimCopy(const std::string& value) {
  const auto start = value.find_first_not_of(" \t\r\n");
  if (start == std::string::npos) {
    return "";
  }
  const auto end = value.find_last_not_of(" \t\r\n");
  return value.substr(start, end - start + 1);
}

void Trim(std::string& value) { value = TrimCopy(value); }

void ToLowerCase(std::string& value) {
  std::transform(value.begin(), value.end(), value.begin(),
                 [](unsigned char c) {
                   return static_cast<char>(std::tolower(c));
                 });
}

bool StartsWith(const std::string& value, const std::string& prefix) {
  return value.size() >= prefix.size() &&
         value.compare(0, prefix.size(), prefix) == 0;
}

std::vector<std::string> SplitQuotedFields(const std::string& value) {
  std::vector<std::string> fields;
  std::string current;
  bool in_quotes = false;

  for (char c : value) {
    if (c == '"') {
      if (in_quotes) {
        fields.push_back(current);
        current.clear();
      }
      in_quotes = !in_quotes;
      continue;
    }

    if (!in_quotes && c == ',') {
      if (!current.empty()) {
        Trim(current);
        fields.push_back(current);
        current.clear();
      }
      continue;
    }

    current.push_back(c);
  }

  if (!current.empty()) {
    Trim(current);
    fields.push_back(current);
  }

  return fields;
}

void SleepMs(int milliseconds) {
  std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

}  // namespace

void SmsModule::SetAction(ActionCallback action) { do_action_ = action; }

void SmsModule::Setup(SerialPort* serial_port) {
  serial_port_ = serial_port;
  network_registered_ = false;
  ResetGsm();
  SleepMs(500);
  serial_port_->Print("AT+CREG?\r");
}

void SmsModule::ParseData(const std::string& data) {
  if (data.empty()) {
    return;
  }

  std::string normalized;
  normalized.reserve(partial_line_.size() + data.size());
  normalized.append(partial_line_);
  partial_line_.clear();

  for (char c : data) {
    if (c == '\r') {
      normalized.push_back('\n');
    } else if (c != '\0') {
      normalized.push_back(c);
    }
  }

  std::size_t start = 0;
  while (start < normalized.size()) {
    const std::size_t end = normalized.find('\n', start);
    if (end == std::string::npos) {
      break;
    }

    std::string line = normalized.substr(start, end - start);
    Trim(line);
    start = end + 1;

    if (line.empty()) {
      continue;
    }

    if (line == "OK" || line == "ERROR") {
      if (pending_sms_header_) {
        pending_sms_header_ = false;
        pending_sms_header_line_.clear();
      }
      continue;
    }

    if (pending_sms_header_) {
      HandleSmsPayload(pending_sms_header_line_, line);
      pending_sms_header_ = false;
      pending_sms_header_line_.clear();
      continue;
    }

    if (line == "RING") {
      HandleRing();
      continue;
    }

    if (StartsWith(line, "+CMTI:")) {
      HandleSmsNotification(line);
      continue;
    }

    if (StartsWith(line, "+CREG:")) {
      HandleNetworkRegistration(line);
      continue;
    }

    if (StartsWith(line, "+CMT:") || StartsWith(line, "+CMGR:")) {
      pending_sms_header_ = true;
      pending_sms_header_line_ = line;
      continue;
    }

    if (StartsWith(line, "+CCLK:")) {
      gsm_time_ = line;
      waiting_for_time_ = false;
      continue;
    }

    if (StartsWith(line, "+CSQ:")) {
      std::printf("GSM Signal strength: %s\n", line.c_str());
      continue;
    }
  }

  if (start < normalized.size()) {
    partial_line_.assign(normalized, start, normalized.size() - start);
  }
}

void SmsModule::ResetGsm() {
  sleep(5);
  serial_port_->Print("AT+CMGF=1\r");
  SleepMs(100);
  serial_port_->Print("AT+CREG=1\r");
  SleepMs(100);
  serial_port_->Print("AT+CMGD=1,4\r");
  SleepMs(100);
  serial_port_->Println("AT+CMGDA= \"DEL ALL\"");
  SleepMs(100);
  Reply("GSM reset successfully", phone_numbers_[0]);
}

bool SmsModule::ValidatePhoneNumber(const char* number) {
  for (int i = 0; i < kTotalPhoneNumbers; ++i) {
    if (std::strncmp(phone_numbers_[i], number, kPhoneNumberSize) == 0) {
      return true;
    }
  }
  return false;
}

void SmsModule::ClearSmsState() {
  sender_number_.clear();
  msg_.clear();
}

void SmsModule::HandleRing() { ResetGsm(); }

void SmsModule::HandleSmsNotification(const std::string& notification) {
  const std::size_t comma = notification.find(',');
  if (comma == std::string::npos) {
    return;
  }

  const std::string index = TrimCopy(notification.substr(comma + 1));
  if (index.empty()) {
    return;
  }

  serial_port_->Print("AT+CMGR=" + index + "\r");
}

void SmsModule::HandleNetworkRegistration(const std::string& notification) {
  std::printf("Received network registration notification: %s\n",
              notification.c_str());
  const std::string payload =
      TrimCopy(notification.substr(std::string("+CREG:").size()));
  const auto fields = SplitQuotedFields(payload);
  std::printf("Parsed fields: %zu\n", fields.size());
  if (fields.size() >= 2) {
    const std::string registration_status = TrimCopy(fields[1]);
    // 0=not registered, 1=registered(home), 2=searching, 3=denied,
    // 4=unknown, 5=registered(roaming)
    std::printf("GSM Network status: %s (registered: %s)\n",
                registration_status.c_str(),
                (registration_status == "1" || registration_status == "5")
                    ? "YES"
                    : "NO");
    network_registered_ =
        (registration_status == "1" || registration_status == "5");
  } else {
    std::printf("Failed to parse network registration response\n");
  }
}

bool SmsModule::ParseSmsHeaderAndBody(const std::string& header,
                                      const std::string& body) {
  const std::size_t colon = header.find(':');
  if (colon == std::string::npos) {
    return false;
  }

  std::string payload = header.substr(colon + 1);
  Trim(payload);
  const auto fields = SplitQuotedFields(payload);

  msg_ = body;
  Trim(msg_);
  ToLowerCase(msg_);

  if (StartsWith(header, "+CMT:")) {
    if (!fields.empty()) {
      sender_number_ = fields[0];
    }
    return !sender_number_.empty();
  }

  if (StartsWith(header, "+CMGR:")) {
    if (fields.size() > 1) {
      sender_number_ = fields[1];
    }
    return !sender_number_.empty();
  }

  return false;
}

void SmsModule::HandleSmsPayload(const std::string& header,
                                 const std::string& body) {
  if (!ParseSmsHeaderAndBody(header, body)) {
    return;
  }

  // Clean SMS storage every time a command is received.
  serial_port_->Print("AT+CMGD=1,4\r");
  SleepMs(150);
  serial_port_->Print("AT+CMGDA= \"DEL ALL\"\r");
  SleepMs(150);

  if (ValidatePhoneNumber(sender_number_.c_str())) {
    if (do_action_ != nullptr) {
      do_action_(sender_number_, msg_);
    }
    ClearSmsState();
  }
}

void SmsModule::Reply(const std::string& text,
                      const std::string& phone_number) {
  serial_port_->Print("AT+CMGF=1\r");
  SleepMs(100);
  serial_port_->Print("AT+CMGS=\"" + phone_number + "\"\r");
  SleepMs(100);
  serial_port_->Print(text);
  SleepMs(200);
  serial_port_->Write(static_cast<char>(0x1A));
  SleepMs(200);
}

void SmsModule::Call(const std::string& phone_number) {
  serial_port_->Print("ATD" + phone_number + ";\r");
}

std::string SmsModule::GetTimeFromGsm() {
  serial_port_->Print("AT+CLTS=1\r");
  SleepMs(500);
  serial_port_->Print("AT&W\r");
  SleepMs(500);
  serial_port_->Print("AT+CFUN=1,1\r");
  SleepMs(4000);

  waiting_for_time_ = true;
  gsm_time_.clear();
  serial_port_->Print("AT+CCLK?\r");
  SleepMs(200);  // Give GSM time to start responding.

  for (int i = 0; i < 100 && waiting_for_time_; ++i) {
    SleepMs(100);
  }
  waiting_for_time_ = false;
  return gsm_time_;
}

bool SmsModule::IsNetworkRegistered() { return network_registered_; }

void SmsModule::CheckSignalStrength() { serial_port_->Print("AT+CSQ\r"); }

void SmsModule::QueryNetworkRegistration() {
  serial_port_->Print("AT+CREG?\r");
}
