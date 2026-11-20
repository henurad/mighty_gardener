#ifndef HARDWARE_GPIO_LINE_H_
#define HARDWARE_GPIO_LINE_H_

class GpioLine {
 public:
  enum class Direction { kInput, kOutput };

  GpioLine(int pin, Direction direction, bool initial_value = false);
  ~GpioLine();

  GpioLine(const GpioLine&) = delete;
  GpioLine& operator=(const GpioLine&) = delete;

  bool Open();
  void Close();
  bool SetDirection(Direction direction, bool output_value = false);
  bool SetValue(bool value);
  bool Read(bool& value);

 private:
  int pin_;
  Direction direction_;
  bool initial_value_;
  void* chip_ = nullptr;
  void* request_ = nullptr;
};

#endif  // HARDWARE_GPIO_LINE_H_
