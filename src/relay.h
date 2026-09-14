
class Relay {
private:
    gpiod_chip* chip;
    gpiod_line* line;
    int pin;
    bool activeLow;

public:
    Relay(int gpioPin, bool active_low);
    void turnOn();
    void turnOff();
    ~Relay();
};