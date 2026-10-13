#ifndef COMMUNICATION_SERIAL_PORT_H_
#define COMMUNICATION_SERIAL_PORT_H_

#include <termios.h>
#include <unistd.h>

#include <atomic>
#include <string>
#include <thread>

class SerialPort {
 public:
  SerialPort();
  virtual ~SerialPort();

  SerialPort(const SerialPort&) = delete;
  SerialPort& operator=(const SerialPort&) = delete;

  bool Open(const char* device_name, int baud_rate);
  void Start();
  void Join();
  ssize_t Write(const char* data, size_t size);
  ssize_t Write(const std::string& msg);
  ssize_t Write(char byte);
  ssize_t Print(const std::string& msg);
  ssize_t Println(const std::string& msg);

  virtual void OnReceived(const char* data, size_t size);

  void Close();
  bool IsOpen() const;

 private:
  speed_t GetBaud(int baud_rate);
  void ReadLoop();

  int fd_;
  std::atomic<bool> running_;
  std::thread read_thread_;
};

#endif  // COMMUNICATION_SERIAL_PORT_H_
