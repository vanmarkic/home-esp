#pragma once

// ESPHome Switch API Mock
// Lightweight stub for unit testing switch/actuator components

#include <cstdint>
#include <string>
#include <vector>

namespace esphome {
namespace switch_ {

/// Base switch class
class Switch {
 public:
  virtual ~Switch() = default;

  /// Turn the switch on
  void turn_on() {
    write_state(true);
  }

  /// Turn the switch off
  void turn_off() {
    write_state(false);
  }

  /// Toggle the switch
  void toggle() {
    write_state(!state);
  }

  /// Publish the current state
  void publish_state(bool new_state) {
    state = new_state;
    state_history_.push_back(new_state);
  }

  /// Get the switch name
  const std::string& get_name() const { return name_; }
  void set_name(const std::string& name) { name_ = name; }

  /// Get/set icon
  const std::string& get_icon() const { return icon_; }
  void set_icon(const std::string& icon) { icon_ = icon; }

  /// Get/set inverted mode
  bool is_inverted() const { return inverted_; }
  void set_inverted(bool inverted) { inverted_ = inverted; }

  /// Current state
  bool state{false};

  // ========== Test Helpers ==========

  /// Get state history (for test assertions)
  const std::vector<bool>& test_get_state_history() const {
    return state_history_;
  }

  /// Get count of state changes
  size_t test_get_state_change_count() const { return state_history_.size(); }

  /// Check if write_state was called
  bool test_was_write_called() const { return write_called_; }

  /// Get last written state
  bool test_get_last_write() const { return last_write_state_; }

  /// Clear history (reset for next test)
  void test_reset() {
    state_history_.clear();
    write_called_ = false;
    state = false;
  }

 protected:
  /// Override this to implement actual hardware control
  virtual void write_state(bool state) {
    write_called_ = true;
    last_write_state_ = state;
    // In a real implementation, this would control hardware
    // By default, just update and publish
    publish_state(inverted_ ? !state : state);
  }

 private:
  std::string name_;
  std::string icon_;
  bool inverted_{false};
  bool write_called_{false};
  bool last_write_state_{false};
  std::vector<bool> state_history_;
};

// Logging helper macro
#define LOG_SWITCH(prefix, type, obj) \
  if (obj != nullptr) { \
    ESP_LOGCONFIG(TAG, "%s%s '%s'", prefix, type, obj->get_name().c_str()); \
  }

}  // namespace switch_
}  // namespace esphome
