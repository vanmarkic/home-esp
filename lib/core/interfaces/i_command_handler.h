#pragma once

// ICommandHandler Interface
// Abstraction for handling actuator commands (switches, outputs)
// Allows business logic to be tested without ESPHome dependencies

namespace home_esp {

class ICommandHandler {
 public:
  virtual ~ICommandHandler() = default;

  /// Execute a command (turn on/off)
  virtual void execute(bool state) = 0;

  /// Get current state
  virtual bool get_state() const = 0;

  /// Toggle current state
  virtual void toggle() {
    execute(!get_state());
  }
};

}  // namespace home_esp
