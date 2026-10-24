#include "control/schmitt-trigger-relay.h"

#include <stdexcept>

SchmittTriggerRelay::SchmittTriggerRelay(Relay& relay, float min, float max)
    : relay_(relay), min_(min), max_(max) {
  if (min_ > max_) {
    throw std::invalid_argument("min must not be greater than max");
  }
}

void SchmittTriggerRelay::Update(float current_value) {
  if (current_value <= min_) {
    SetOn(true);
  } else if (current_value >= max_) {
    SetOn(false);
  }
}

bool SchmittTriggerRelay::IsOn() const { return is_on_; }

void SchmittTriggerRelay::SetOn(bool on) {
  if (on == is_on_) {
    return;
  }

  is_on_ = on;
  if (on) {
    relay_.TurnOn();
  } else {
    relay_.TurnOff();
  }
}
