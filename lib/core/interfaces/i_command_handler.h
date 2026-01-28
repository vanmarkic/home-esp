#pragma once

/// @file i_command_handler.h
/// @brief Interface for handling actuator commands (switches, outputs)
///
/// This interface abstracts the hardware/platform-specific command execution,
/// allowing business logic to be tested without ESPHome dependencies.
///
/// ## Implementations:
/// - **ESPHomeSwitchAdapter**: Bridges to ESPHome switch component
/// - **MockCommandHandler**: Test double for unit testing
///
/// ## Usage Pattern:
/// @code
///   // Production code
///   ESPHomeSwitchAdapter adapter(esphome_switch);
///   RelayController controller(&adapter, config);
///
///   // Test code
///   MockCommandHandler mock;
///   RelayController controller(&mock, config);
///   controller.turn_on();
///   EXPECT_TRUE(mock.get_state());
/// @endcode

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
