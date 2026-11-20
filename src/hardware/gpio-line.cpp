#include "hardware/gpio-line.h"

#ifdef HAVE_GPIOD
#include <gpiod.h>
#endif

#include <iostream>

GpioLine::GpioLine(int pin, Direction direction, bool initial_value)
    : pin_(pin),
      direction_(direction),
      initial_value_(initial_value) {}

GpioLine::~GpioLine() { Close(); }

bool GpioLine::Open() {
#ifndef HAVE_GPIOD
  (void)pin_;
  std::cerr << "libgpiod support is unavailable; GPIO operation is disabled.\n";
  return false;
#else
  if (request_ != nullptr) {
    return true;
  }

  chip_ = gpiod_chip_open("/dev/gpiochip0");
  if (chip_ == nullptr) {
    std::cerr << "Failed to open /dev/gpiochip0\n";
    return false;
  }

  gpiod_line_settings* settings = gpiod_line_settings_new();
  gpiod_request_config* request_config = gpiod_request_config_new();
  gpiod_line_config* line_config = gpiod_line_config_new();
  if (settings == nullptr || request_config == nullptr || line_config == nullptr) {
    gpiod_line_settings_free(settings);
    gpiod_request_config_free(request_config);
    gpiod_line_config_free(line_config);
    Close();
    return false;
  }

  gpiod_request_config_set_consumer(request_config, "mighty-gardener");
  gpiod_line_settings_set_direction(
      settings,
      direction_ == Direction::kOutput
          ? GPIOD_LINE_DIRECTION_OUTPUT
          : GPIOD_LINE_DIRECTION_INPUT);
  gpiod_line_settings_set_output_value(
      settings,
      initial_value_ ? GPIOD_LINE_VALUE_ACTIVE
                     : GPIOD_LINE_VALUE_INACTIVE);

  const unsigned int offset = static_cast<unsigned int>(pin_);
  if (gpiod_line_config_add_line_settings(
          line_config, &offset, 1, settings) < 0) {
    std::cerr << "Failed to configure GPIO line " << pin_ << "\n";
  } else {
    request_ = gpiod_chip_request_lines(
        reinterpret_cast<gpiod_chip*>(chip_), request_config, line_config);
  }

  gpiod_line_settings_free(settings);
  gpiod_request_config_free(request_config);
  gpiod_line_config_free(line_config);

  if (request_ == nullptr) {
    std::cerr << "Failed to request GPIO line " << pin_ << "\n";
    gpiod_chip_close(reinterpret_cast<gpiod_chip*>(chip_));
    chip_ = nullptr;
    return false;
  }
  return true;
#endif
}

void GpioLine::Close() {
#ifdef HAVE_GPIOD
  if (request_ != nullptr) {
    gpiod_line_request_release(reinterpret_cast<gpiod_line_request*>(request_));
    request_ = nullptr;
  }
  if (chip_ != nullptr) {
    gpiod_chip_close(reinterpret_cast<gpiod_chip*>(chip_));
    chip_ = nullptr;
  }
#endif
}

bool GpioLine::SetDirection(Direction direction, bool output_value) {
  direction_ = direction;
  initial_value_ = output_value;
  Close();
  return Open();
}

bool GpioLine::SetValue(bool value) {
#ifdef HAVE_GPIOD
  if (request_ == nullptr && !Open()) {
    return false;
  }
  return gpiod_line_request_set_value(
             reinterpret_cast<gpiod_line_request*>(request_),
             static_cast<unsigned int>(pin_),
             value ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE) == 0;
#else
  (void)value;
  return false;
#endif
}

bool GpioLine::Read(bool& value) {
#ifdef HAVE_GPIOD
  if (request_ == nullptr && !Open()) {
    return false;
  }
  gpiod_line_value line_value = gpiod_line_request_get_value(
      reinterpret_cast<gpiod_line_request*>(request_),
      static_cast<unsigned int>(pin_));
  if (line_value == GPIOD_LINE_VALUE_ERROR) {
    return false;
  }
  value = line_value == GPIOD_LINE_VALUE_ACTIVE;
  return true;
#else
  (void)value;
  return false;
#endif
}
