#pragma once

// ESPHome Binary Sensor API Mock
// Lightweight stub for unit testing binary sensor components

#include <cstdint>
#include <string>
#include <vector>

namespace esphome {
namespace binary_sensor {

/// Device classes for binary sensors
enum BinaryDeviceClass : uint8_t {
  DEVICE_CLASS_NONE = 0,
  DEVICE_CLASS_BATTERY,
  DEVICE_CLASS_COLD,
  DEVICE_CLASS_CONNECTIVITY,
  DEVICE_CLASS_DOOR,
  DEVICE_CLASS_GARAGE_DOOR,
  DEVICE_CLASS_GAS,
  DEVICE_CLASS_HEAT,
  DEVICE_CLASS_LIGHT,
  DEVICE_CLASS_LOCK,
  DEVICE_CLASS_MOISTURE,
  DEVICE_CLASS_MOTION,
  DEVICE_CLASS_MOVING,
  DEVICE_CLASS_OCCUPANCY,
  DEVICE_CLASS_OPENING,
  DEVICE_CLASS_PLUG,
  DEVICE_CLASS_POWER,
  DEVICE_CLASS_PRESENCE,
  DEVICE_CLASS_PROBLEM,
  DEVICE_CLASS_RUNNING,
  DEVICE_CLASS_SAFETY,
  DEVICE_CLASS_SMOKE,
  DEVICE_CLASS_SOUND,
  DEVICE_CLASS_TAMPER,
  DEVICE_CLASS_UPDATE,
  DEVICE_CLASS_VIBRATION,
  DEVICE_CLASS_WINDOW,
};

/// Base binary sensor class
class BinarySensor {
 public:
  virtual ~BinarySensor() = default;

  /// Publish a new state
  void publish_state(bool new_state) {
    state = new_state;
    has_state_ = true;
    state_history_.push_back(new_state);
  }

  /// Publish initial state (won't trigger if same as current)
  void publish_initial_state(bool new_state) {
    if (!has_state_) {
      publish_state(new_state);
    }
  }

  /// Check if sensor has a valid state
  bool has_state() const { return has_state_; }

  /// Get the sensor's name
  const std::string& get_name() const { return name_; }
  void set_name(const std::string& name) { name_ = name; }

  /// Get/set device class
  BinaryDeviceClass get_device_class() const { return device_class_; }
  void set_device_class(BinaryDeviceClass device_class) {
    device_class_ = device_class;
  }

  /// Get/set icon
  const std::string& get_icon() const { return icon_; }
  void set_icon(const std::string& icon) { icon_ = icon; }

  /// Get/set inverted
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
  size_t test_get_publish_count() const { return state_history_.size(); }

  /// Get transitions (true->false or false->true)
  int test_count_transitions() const {
    if (state_history_.size() < 2) return 0;
    int transitions = 0;
    for (size_t i = 1; i < state_history_.size(); ++i) {
      if (state_history_[i] != state_history_[i - 1]) {
        transitions++;
      }
    }
    return transitions;
  }

  /// Clear history (reset for next test)
  void test_reset() {
    state_history_.clear();
    state = false;
    has_state_ = false;
  }

 private:
  std::string name_;
  std::string icon_;
  BinaryDeviceClass device_class_{DEVICE_CLASS_NONE};
  bool inverted_{false};
  bool has_state_{false};
  std::vector<bool> state_history_;
};

// Logging helper macro
#define LOG_BINARY_SENSOR(prefix, type, sensor) \
  if (sensor != nullptr) { \
    ESP_LOGCONFIG(TAG, "%s%s '%s'", prefix, type, sensor->get_name().c_str()); \
  }

}  // namespace binary_sensor
}  // namespace esphome
