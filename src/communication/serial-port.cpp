#include "communication/serial-port.h"

#include <fcntl.h>
#include <termios.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <iostream>

SerialPort::SerialPort() : fd_(-1), running_(false) {}

SerialPort::~SerialPort() { Close(); }

speed_t SerialPort::GetBaud(int baud_rate) {
  switch (baud_rate) {
    case 9600:
      return B9600;
    case 19200:
      return B19200;
    case 38400:
      return B38400;
    case 57600:
      return B57600;
    case 115200:
      return B115200;
    default:
      return B9600;
  }
}

void SerialPort::ReadLoop() {
  char buffer[256];

  while (running_.load()) {
    const ssize_t n = ::read(fd_, buffer, sizeof(buffer));
    if (n > 0) {
      OnReceived(buffer, n);
    } else if (n == 0) {
      break;
    } else {
      if (errno == EINTR) {
        continue;
      }
      break;
    }
  }
}

void SerialPort::Start() {
  if (fd_ < 0) {
    return;
  }

  if (running_.load()) {
    return;
  }

  running_ = true;
  read_thread_ = std::thread(&SerialPort::ReadLoop, this);
}

void SerialPort::Join() {
  if (read_thread_.joinable()) {
    read_thread_.join();
  }
}

bool SerialPort::Open(const char* device_name, int baud_rate) {
  if (fd_ >= 0) {
    Close();
  }

  fd_ = ::open(device_name, O_RDWR | O_NOCTTY | O_SYNC);
  if (fd_ < 0) {
    std::perror("open");
    return false;
  }

  termios tty{};

  if (tcgetattr(fd_, &tty) != 0) {
    std::perror("tcgetattr");
    Close();
    return false;
  }

  cfsetospeed(&tty, GetBaud(baud_rate));
  cfsetispeed(&tty, GetBaud(baud_rate));

  tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
  tty.c_iflag &= ~IGNBRK;
  tty.c_lflag = 0;
  tty.c_oflag = 0;

  tty.c_cc[VMIN] = 1;
  tty.c_cc[VTIME] = 0;

  tty.c_iflag &= ~(IXON | IXOFF | IXANY);
  tty.c_cflag |= (CLOCAL | CREAD);
  tty.c_cflag &= ~(PARENB | PARODD);
  tty.c_cflag &= ~CSTOPB;
  tty.c_cflag &= ~CRTSCTS;

  if (tcsetattr(fd_, TCSANOW, &tty) != 0) {
    std::perror("tcsetattr");
    Close();
    return false;
  }

  Start();
  return true;
}

ssize_t SerialPort::Write(const char* data, size_t size) {
  if (fd_ < 0) {
    return -1;
  }
  return ::write(fd_, data, size);
}

ssize_t SerialPort::Write(const std::string& msg) {
  return Write(msg.c_str(), msg.size());
}

ssize_t SerialPort::Write(char byte) { return Write(&byte, 1); }

ssize_t SerialPort::Print(const std::string& msg) { return Write(msg); }

ssize_t SerialPort::Println(const std::string& msg) {
  const std::string line = msg + "\r";
  return Write(line);
}

void SerialPort::OnReceived(const char* data, size_t size) {
  std::cout << "Received (" << size << " bytes): ";
  std::cout.write(data, size);
  std::cout << std::endl;
}

void SerialPort::Close() {
  if (running_.load()) {
    running_ = false;
    Join();
  }

  if (fd_ >= 0) {
    ::close(fd_);
    fd_ = -1;
  }
}

bool SerialPort::IsOpen() const { return fd_ >= 0; }
