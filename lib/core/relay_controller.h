#pragma once

// RelayController - Example business logic for actuators
// Pure C++ with no ESPHome dependencies
// Controls a relay with optional timing constraints

#include "interfaces/i_command_handler.h"
#include <cstdint>

namespace home_esp {

class RelayController {
 public:
  /// Configuration for relay behavior
  struct Config {
    uint32_t min_on_time_ms = 0;     // Minimum time to stay on (protection)
    uint32_t min_off_time_ms = 0;    // Minimum time to stay off (protection)
    bool inverted = false;           // Invert output logic
    bool restore_state = false;      // Restore state on boot
  };

  explicit RelayController(ICommandHandler* handler, Config config = Config())
      : handler_(handler), config_(config) {}

  /// Request to turn on
  /// @return true if command was executed, false if blocked by timing
  bool turn_on() {
    return execute_command(true);
  }

  /// Request to turn off
  /// @return true if command was executed, false if blocked by timing
  bool turn_off() {
    return execute_command(false);
  }

  /// Toggle current state
  /// @return true if command was executed, false if blocked by timing
  bool toggle() {
    return execute_command(!current_state_);
  }

  /// Get current state
  bool is_on() const { return current_state_; }

  /// Update timing (call this regularly with current millis)
  void update(uint32_t current_millis) {
    current_millis_ = current_millis;
  }

  /// Get configuration
  const Config& get_config() const { return config_; }

 private:
  bool execute_command(bool requested_state) {
    // Check if we're allowed to change state based on timing
    if (!can_change_state(requested_state)) {
      return false;
    }

    // Apply inversion if configured
    bool output_state = config_.inverted ? !requested_state : requested_state;

    // Execute the command
    handler_->execute(output_state);

    // Update internal state
    current_state_ = requested_state;
    last_change_millis_ = current_millis_;

    return true;
  }

  bool can_change_state(bool requested_state) const {
    if (requested_state == current_state_) {
      return true;  // No change needed
    }

    uint32_t elapsed = current_millis_ - last_change_millis_;

    if (current_state_) {
      // Currently ON, wanting to turn OFF
      return elapsed >= config_.min_on_time_ms;
    } else {
      // Currently OFF, wanting to turn ON
      return elapsed >= config_.min_off_time_ms;
    }
  }

  ICommandHandler* handler_;
  Config config_;
  bool current_state_{false};
  uint32_t current_millis_{0};
  uint32_t last_change_millis_{0};
};

}  // namespace home_esp
