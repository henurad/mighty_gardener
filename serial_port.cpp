#include "serial_port.h"

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <termios.h>

SerialPort::SerialPort() : fd(-1), running(false) {}

SerialPort::~SerialPort() {
    close();
}

speed_t SerialPort::getBaud(int baud_rate) {
    switch (baud_rate) {
        case 9600: return B9600;
        case 19200: return B19200;
        case 38400: return B38400;
        case 57600: return B57600;
        case 115200: return B115200;
        default: return B9600;
    }
}

void SerialPort::readLoop() {
    char buffer[256];

    while (running.load()) {
        ssize_t n = ::read(fd, buffer, sizeof(buffer));
        if (n > 0) {
            on_received(buffer, n);
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

bool SerialPort::open(const char* device_name, int baud_rate) {
    if (fd >= 0) {
        close();
    }

    fd = ::open(device_name, O_RDWR | O_NOCTTY | O_SYNC);
    if (fd < 0) {
        perror("open");
        return false;
    }

    struct termios tty;
    memset(&tty, 0, sizeof tty);

    if (tcgetattr(fd, &tty) != 0) {
        perror("tcgetattr");
        close();
        return false;
    }

    cfsetospeed(&tty, getBaud(baud_rate));
    cfsetispeed(&tty, getBaud(baud_rate));

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

    if (tcsetattr(fd, TCSANOW, &tty) != 0) {
        perror("tcsetattr");
        close();
        return false;
    }

    running = true;
    readThread = std::thread(&SerialPort::readLoop, this);
    return true;
}

ssize_t SerialPort::write(const char* data, size_t size) {
    if (fd < 0) return -1;
    return ::write(fd, data, size);
}

ssize_t SerialPort::write(const std::string& msg) {
    return write(msg.c_str(), msg.size());
}

ssize_t SerialPort::write(char byte) {
    return write(&byte, 1);
}

ssize_t SerialPort::print(const std::string& msg) {
    return write(msg);
}

ssize_t SerialPort::println(const std::string& msg) {
    std::string line = msg + "\r";
    return write(line);
}

void SerialPort::on_received(const char* data, size_t size) {
    std::cout << "Received (" << size << " bytes): ";
    std::cout.write(data, size);
    std::cout << std::endl;
}

void SerialPort::close() {
    if (running.load()) {
        running = false;
        if (readThread.joinable()) {
            readThread.join();
        }
    }

    if (fd >= 0) {
        ::close(fd);
        fd = -1;
    }
}

bool SerialPort::isOpen() const {
    return fd >= 0;
}
