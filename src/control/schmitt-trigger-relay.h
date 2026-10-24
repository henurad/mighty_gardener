#ifndef CONTROL_SCHMITT_TRIGGER_RELAY_H_
#define CONTROL_SCHMITT_TRIGGER_RELAY_H_

#include "hardware/relay.h"

/// @brief Drives a relay with Schmitt-trigger (hysteresis) behaviour.
///
/// The relay turns on when the current value reaches the minimum threshold or
/// below, and turns off when it reaches the maximum threshold or above. Values
/// between the thresholds keep the relay in its current state.
class SchmittTriggerRelay {
 public:
  SchmittTriggerRelay(Relay& relay, float min, float max);

  SchmittTriggerRelay(const SchmittTriggerRelay&) = delete;
  SchmittTriggerRelay& operator=(const SchmittTriggerRelay&) = delete;

  /// @brief Applies a new current value and switches the relay if needed.
  void Update(float current_value);

  /// @brief Returns whether the relay is currently on.
  bool IsOn() const;

  float min() const { return min_; }
  float max() const { return max_; }

 private:
  void SetOn(bool on);

  Relay& relay_;
  float min_;
  float max_;
  bool is_on_ = false;
};

#endif  // CONTROL_SCHMITT_TRIGGER_RELAY_H_
