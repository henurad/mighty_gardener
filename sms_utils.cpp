#include "sms_utils.h"
#include <cstdio>
#include <unistd.h>

void SmsModule::SetAction(void (*action)(std::string, std::string)) {
    do_action = action;
}

void SmsModule::Setup(SerialPort* serial_port) {
    this->serial_port = serial_port;
    network_registered = false;  // Reset flag
    ResetGsm();
    // Query current network registration status
    msleep(500);
    serial_port->print("AT+CREG?\r");
}

void SmsModule::ParseData(const std::string& buff) {
    if (buff.empty()) {
        return;
    }

    std::string normalized;
    normalized.reserve(partial_line.size() + buff.size());
    normalized.append(partial_line);
    partial_line.clear();

    for (char c : buff) {
        if (c == '\r') {
            normalized.push_back('\n');
        } else if (c != '\0') {
            normalized.push_back(c);
        }
    }

    std::size_t start = 0;
    while (start < normalized.size()) {
        std::size_t end = normalized.find('\n', start);
        if (end == std::string::npos) {
            break;
        }

        std::string line = normalized.substr(start, end - start);
        trim(line);
        start = end + 1;

        // Skip empty lines, but if we're waiting for SMS body, don't skip non-empty lines
        if (line.empty()) {
            continue;  // Always skip empty lines
        }

        if (line == "OK" || line == "ERROR") {
            if (pending_sms_header) {
                // Stop waiting for SMS body if we get OK/ERROR
                pending_sms_header = false;
                pending_sms_header_line.clear();
            }
            continue;  // Skip OK/ERROR
        }

        if (pending_sms_header) {
            HandleSmsPayload(pending_sms_header_line, line);
            pending_sms_header = false;
            pending_sms_header_line.clear();
            continue;
        }

        if (line == "RING") {
            HandleRing();
            continue;
        }

        if (startsWith(line, "+CMTI:")) {
            HandleSmsNotification(line);
            continue;
        }

        if (startsWith(line, "+CREG:")) {
            HandleNetworkRegistration(line);
            continue;
        }

        if (startsWith(line, "+CLIP:")) {
            HandleClip(line);
            continue;
        }

        if (startsWith(line, "+CMT:") || startsWith(line, "+CMGR:")) {
            pending_sms_header = true;
            pending_sms_header_line = line;
            continue;
        }

        if (startsWith(line, "+CCLK:")) {
            gsm_time = line;
            waiting_for_time = false;
            continue;
        }

        if (startsWith(line, "+CSQ:")) {
            printf("GSM Signal strength: %s\n", line.c_str());
            continue;
        }
    }

    if (start < normalized.size()) {
        partial_line.assign(normalized, start, normalized.size() - start);
    }
}

void SmsModule::ResetGsm() {
    // digitalWrite(kGsmResetPin, LOW);
    // digitalWrite(kGsmResetPin, HIGH);

    sleep(5);
    serial_port->print("AT+CMGF=1\r");
    msleep(100);
    serial_port->print("AT+CREG=1\r");  // Enable network registration reporting
    msleep(100);
    serial_port->print("AT+CMGD=1,4\r");
    msleep(100);
    serial_port->println("AT+CMGDA= \"DEL ALL\"");
    msleep(100);
    Reply("Gsm reseted successfully", phone_numbers[0]);
}

bool SmsModule::ValidatePhoneNumber(const char* number) {
    for (int i = 0; i < kTotalPhoneNo; i++) {
        if (strncmp(phone_numbers[i], number, kPhoneNumberSize) == 0) {
            return true;
        }
    }
    return false;
}

void SmsModule::ClearSmsState() {
    sms_status.clear();
    sender_number.clear();
    recieved_date.clear();
    msg.clear();
}

void SmsModule::HandleRing() {
    ResetGsm();
}

void SmsModule::HandleSmsNotification(const std::string& notification) {
    std::size_t comma = notification.find(',');
    if (comma == std::string::npos) {
        return;
    }

    std::string index = trimCopy(notification.substr(comma + 1));
    if (index.empty()) {
        return;
    }

    serial_port->print("AT+CMGR=" + index + "\r");
}

void SmsModule::HandleNetworkRegistration(const std::string& notification) {
    printf("Received network registration notification: %s\n", notification.c_str());
    std::string payload = trimCopy(notification.substr(std::string("+CREG:").size()));
    auto fields = splitQuotedFields(payload);
    printf("Parsed fields: %zu\n", fields.size());
    if (fields.size() >= 2) {
        std::string registration_status = trimCopy(fields[1]);
        // registration_status: 0=not registered, 1=registered(home), 2=searching,
        // 3=denied, 4=unknown, 5=registered(roaming)
        printf("GSM Network status: %s (registered: %s)\n", registration_status.c_str(), 
               (registration_status == "1" || registration_status == "5") ? "YES" : "NO");
        network_registered = (registration_status == "1" || registration_status == "5");
    } else {
        printf("Failed to parse network registration response\n");
    }
}

void SmsModule::HandleClip(const std::string& notification) {
    std::string payload = trimCopy(notification.substr(std::string("+CLIP:").size()));
    auto fields = splitQuotedFields(payload);
    if (!fields.empty()) {
        std::string caller_id = trimCopy(fields[0]);
        (void)caller_id;
    }
}

bool SmsModule::ParseSmsHeaderAndBody(const std::string& header, const std::string& body) {
    std::size_t colon = header.find(':');
    if (colon == std::string::npos) {
        return false;
    }

    std::string payload = header.substr(colon + 1);
    trim(payload);
    auto fields = splitQuotedFields(payload);

    msg = body;
    trim(msg);
    toLowerCase(msg);

    if (startsWith(header, "+CMT:")) {
        if (!fields.empty()) {
            sender_number = fields[0];
        }
        if (fields.size() > 2) {
            recieved_date = fields[2];
        }
        return !sender_number.empty();
    }

    if (startsWith(header, "+CMGR:")) {
        if (!fields.empty()) {
            sms_status = fields[0];
        }
        if (fields.size() > 1) {
            sender_number = fields[1];
        }
        if (fields.size() > 3) {
            recieved_date = fields[3];
        }
        return !sender_number.empty();
    }

    return false;
}

void SmsModule::HandleSmsPayload(const std::string& header, const std::string& body) {
    if (!ParseSmsHeaderAndBody(header, body)) {
        return;
    }

    // Clean SMS storage every time a command is received
    serial_port->print("AT+CMGD=1,4\r");
    msleep(150);
    serial_port->print("AT+CMGDA= \"DEL ALL\"\r");
    msleep(150);

    if (ValidatePhoneNumber(sender_number.c_str())) {
        if (do_action) {
            do_action(sender_number, msg);
        }

        ClearSmsState();
    }
}

void SmsModule::Reply(const std::string& text, const std::string& Phone) {

    serial_port->print("AT+CMGF=1\r");
    msleep(100);
    serial_port->print("AT+CMGS=\"" + Phone + "\"\r");
    msleep(100);
    serial_port->print(text);
    msleep(200);
    serial_port->write(static_cast<char>(0x1A));
    msleep(200);
}

void SmsModule::Call(const std::string& Phone) {
    serial_port->print("ATD" + Phone + ";\r");
}

std::string SmsModule::GetTimeFromGsm() {
    serial_port->print("AT+CLTS=1\r");
    msleep(500);
    serial_port->print("AT&W\r");
    msleep(500);
    serial_port->print("AT+CFUN=1,1\r");
    msleep(4000);

    waiting_for_time = true;
    gsm_time = "";
    serial_port->print("AT+CCLK?\r");
    msleep(200);  // Give GSM time to start responding
    
    // Wait for response with timeout (up to 10 seconds)
    for (int i = 0; i < 100 && waiting_for_time; ++i) {
        msleep(100);
    }
    waiting_for_time = false;
    return gsm_time;
}

bool SmsModule::IsNetworkRegistered() {
    return network_registered;
}

void SmsModule::CheckSignalStrength() {
    checking_signal = true;
    serial_port->print("AT+CSQ\r");
    msleep(200);
    checking_signal = false;
}

void SmsModule::QueryNetworkRegistration() {
    serial_port->print("AT+CREG?\r");
}
