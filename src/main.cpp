#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <thread>

#include "communication/serial-port.h"
#include "communication/sms-manager.h"
#include "config/app-config.h"
#include "control/relay-scheduler.h"
#include "hardware/dht11-sensor.h"
#include "hardware/relay.h"

namespace {

SmsManager* g_sms_manager = nullptr;

class GsmSerialPort : public SerialPort {
 public:
  void OnReceived(const char* data, size_t size) override {
    const std::string str(data, size);
    std::printf("%s", str.c_str());
    if (g_sms_manager != nullptr) {
      g_sms_manager->ParseData(str);
    }
  }
};

void DoAction(const std::string& phone_number, const std::string& msg) {
  if (msg == "callme") {
    g_sms_manager->Call(phone_number);
  } else if (msg == "ping") {
    g_sms_manager->SendReply("pong", phone_number);
  } else if (msg == "rstgsm") {
    g_sms_manager->ResetGsm();
  } else if (msg == "v1 on") {
    g_sms_manager->SendReply("valve1 opened", phone_number);
  } else if (msg == "v1 off") {
    g_sms_manager->SendReply("valve1 closed", phone_number);
  } else if (msg == "v2 on") {
    g_sms_manager->SendReply("valve2 opened", phone_number);
  } else if (msg == "v2 off") {
    g_sms_manager->SendReply("valve2 closed", phone_number);
  } else if (msg == "status") {
    // TODO: report system status.
  } else if (msg == "reboot") {
    g_sms_manager->SendReply("Rebooting system...", phone_number);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::system("sudo reboot");
  } else if (msg == "help") {
    const std::string help_message =
        "Available commands:\n"
        "callme - Call your number\n"
        "ping - Check if system is responsive\n"
        "rstgsm - Reset GSM module\n"
        "v1 on/off - Open/Close valve 1\n"
        "v2 on/off - Open/Close valve 2\n"
        "status - Get system status\n"
        "reboot - Reboot the system\n"
        "help - Show this help message";
    g_sms_manager->SendReply(help_message, phone_number);
  } else {
    g_sms_manager->SendReply("Unknown command: " + msg, phone_number);
  }
}

}  // namespace

int main() {
  const AppConfig config = LoadAppConfig();

  GsmSerialPort serial_port;
  if (!serial_port.Open(config.serial_port.c_str(), config.baud_rate)) {
    return 1;
  }

  SmsManager sms_manager;
  g_sms_manager = &sms_manager;
  sms_manager.Setup(&serial_port);
  sms_manager.SetAction(DoAction);
  sms_manager.RunStartupSync();

  Relay sun_light_relay(config.sun_light_relay_gpio, true);
  Relay heater_relay(config.heater_relay_gpio, true);
  Relay watering_relay(config.watering_relay_gpio, true);
  RelayScheduler relay_scheduler(serial_port, sun_light_relay, heater_relay,
                                 watering_relay);
  relay_scheduler.Start();

  Dht11Sensor dht(config.dht_gpio);
  if (!dht.Initialize()) {
    return 1;
  }

  dht.StartMonitoring(5);

  while (serial_port.IsOpen()) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  dht.StopMonitoring();
  dht.Join();
  dht.Shutdown();
  relay_scheduler.Join();

  g_sms_manager = nullptr;
  return 0;
}
