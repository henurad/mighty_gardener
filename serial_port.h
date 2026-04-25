#ifndef SERIAL_PORT_H_
#define SERIAL_PORT_H_

#include <atomic>
#include <string>
#include <thread>
#include <unistd.h>
#include <termios.h>

class SerialPort {
private:
    int fd;
    std::atomic<bool> running;
    std::thread readThread;

    speed_t getBaud(int baud_rate);
    void readLoop();

public:
    SerialPort();
    virtual ~SerialPort();

    bool open(const char* device_name, int baud_rate);
    ssize_t write(const char* data, size_t size);
    ssize_t write(const std::string& msg);
    ssize_t write(char byte);
    ssize_t print(const std::string& msg);
    ssize_t println(const std::string& msg);

    virtual void on_received(const char* data, size_t size);
    void close();
    bool isOpen() const;
};

#endif  // SERIAL_PORT_H_
